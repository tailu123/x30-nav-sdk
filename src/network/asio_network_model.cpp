#include "asio_network_model.hpp"
#include "../protocol/x30_protocol.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace network {

AsioNetworkModel::AsioNetworkModel(INetworkCallback& callback)
    : socket_(io_context_),
      strand_(io_context_),
      connected_(false),
      callback_(callback) {
}

AsioNetworkModel::~AsioNetworkModel() {
    disconnect();
}

bool AsioNetworkModel::connect(const std::string& host, uint16_t port) {
    if (connected_) {
        return true;
    }

    try {
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        boost::system::error_code error;
        boost::asio::connect(socket_, endpoints, error);

        if (error) {
            std::cerr << "连接失败: " << error.message() << std::endl;
            return false;
        }

        connected_ = true;

        // 启动IO线程
        io_thread_ = std::thread(&AsioNetworkModel::ioThreadFunc, this);

        // 启动接收
        startReceive();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "连接异常: " << e.what() << std::endl;
        return false;
    }
}

void AsioNetworkModel::disconnect() {
    if (!connected_ && !socket_.is_open()) {
        return;
    }

    try {
        // 取消所有异步操作
        boost::system::error_code ec;
        socket_.cancel(ec);

        // 使用 strand 确保安全关闭
        boost::asio::post(strand_, [this]() {
            // 关闭socket
            boost::system::error_code error;
            socket_.close(error);

            if (error) {
                std::cerr << "关闭socket错误: " << error.message() << std::endl;
            }
        });

        // 停止IO上下文
        io_context_.stop();

        // 等待IO线程结束
        if (io_thread_.joinable()) {
            io_thread_.join();
        }

        connected_ = false;
    } catch (const std::exception& e) {
        std::cerr << "断开连接异常: " << e.what() << std::endl;
    }
}

bool AsioNetworkModel::isConnected() const {
    return connected_ && socket_.is_open();
}

bool AsioNetworkModel::sendMessage(const protocol::IMessage& message) {
    if (!isConnected()) {
        return false;
    }

    try {
        // 序列化消息
        protocol::X30Protocol protocol;
        std::string data = protocol.serializeMessage(message);

        // 使用 strand 包装异步写入操作，确保线程安全
        boost::asio::post(strand_, [this, data = std::move(data)]() {
            if (!isConnected()) {
                return;
            }

            boost::asio::async_write(
                socket_,
                boost::asio::buffer(data),
                boost::asio::bind_executor(strand_,
                    [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                        handleSend(error, bytes_transferred);
                    }
                )
            );
        });

        return true;
    } catch (const std::exception& e) {
        std::cerr << "发送消息异常: " << e.what() << std::endl;
        return false;
    }
}

void AsioNetworkModel::startReceive() {
    if (!isConnected()) {
        return;
    }

    // 使用 strand 包装异步读取操作，确保线程安全
    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_),
        boost::asio::bind_executor(strand_,
            [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReceive(error, bytes_transferred);
            }
        )
    );
}

/**
 * @brief 安全回调包装函数，用于捕获和处理回调函数中可能抛出的异常
 * @tparam Callback 回调函数类型
 * @tparam Args 回调函数参数类型
 * @param callback 回调函数
 * @param callbackType 回调函数类型描述，用于日志记录
 * @param args 回调函数参数
 */
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    try {
        callback(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数异常: " << e.what() << std::endl;
    } catch (...) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数发生未知异常" << std::endl;
    }
}

void AsioNetworkModel::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        if (error != boost::asio::error::operation_aborted) {
            std::cerr << "接收数据错误: " << error.message() << std::endl;
            disconnect();
        }
        return;
    }

    // 将接收到的数据追加到缓冲区
    receive_data_.append(receive_buffer_.data(), bytes_transferred);

    // 尝试解析消息
    protocol::X30Protocol protocol;
    auto message = protocol.parseReceivedData(receive_data_);
    if (message) {
        // 清空接收缓冲区
        receive_data_.clear();

        // 使用 strand 确保回调在同一线程上下文中执行
        boost::asio::post(strand_, [this, msg = std::move(message)]() mutable {
            safeCallback(
                [this](std::unique_ptr<protocol::IMessage>& msg) {
                    callback_.onMessageReceived(std::move(msg));
                },
                "网络消息接收",
                msg
            );
        });
    }

    // 继续接收
    startReceive();
}

void AsioNetworkModel::handleSend(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        std::cerr << "发送数据错误: " << error.message() << std::endl;
        if (error != boost::asio::error::operation_aborted) {
            disconnect();
        }
    }
}

void AsioNetworkModel::ioThreadFunc() {
    try {
        // 使用 work guard 防止 io_context 在没有任务时退出
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context_.get_executor());

        // 重置 io_context 以确保它可以重新运行
        io_context_.restart();

        // 运行 io_context
        io_context_.run();
    } catch (const std::exception& e) {
        std::cerr << "IO线程异常: " << e.what() << std::endl;
    }
}

} // namespace network
