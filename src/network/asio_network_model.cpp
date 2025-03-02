#include "asio_network_model.hpp"
#include "../protocol/x30_protocol.hpp"
#include <iostream>

namespace network {

AsioNetworkModel::AsioNetworkModel(MessageQueue& message_queue)
    : socket_(io_context_),
      connected_(false),
      message_queue_(message_queue) {
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
    if (!connected_) {
        return;
    }

    try {
        // 关闭socket
        boost::system::error_code error;
        socket_.close(error);

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

        // 发送数据
        std::lock_guard<std::mutex> lock(mutex_);
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data),
            [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleSend(error, bytes_transferred);
            }
        );

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

    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_),
        [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
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

        // 将消息推入队列
        message_queue_.pushMessage(std::move(message));
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
        io_context_.run();
    } catch (const std::exception& e) {
        std::cerr << "IO线程异常: " << e.what() << std::endl;
    }
}

} // namespace network
