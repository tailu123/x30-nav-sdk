#include "network/asio_network_model.hpp"
// #include <fmt/core.h>
#include <spdlog/spdlog.h>
#include "common/event_bus.hpp"
#include "common/message_queue.hpp"
#include "common/utils.hpp"
namespace network {
AsioNetworkModel::AsioNetworkModel(common::MessageQueue& message_queue)
    : io_context_(),
      io_thread_([this]() { io_context_.run(); }),
      work_(io_context_),
      socket_(io_context_),
      strand_(io_context_.get_executor()),
      message_queue_(message_queue) {
}

AsioNetworkModel::~AsioNetworkModel() {
    disconnect();
}

bool AsioNetworkModel::connect(const std::string& host, uint16_t port) {
    boost::asio::ip::tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    return doConnect(*endpoints.begin());
}

void AsioNetworkModel::disconnect() {
    // 1. 首先取消所有待处理的异步操作
    if (socket_.is_open()) {
        boost::system::error_code ec;
        socket_.cancel(ec);  // 取消所有待处理的异步操作
        socket_.close(ec);   // 关闭socket
    }

    // 2. 停止IO上下文
    io_context_.stop();
}

bool AsioNetworkModel::isConnected() const {
    return socket_.is_open();
}

void AsioNetworkModel::sendMessage(const protocol::IMessage& message) {
    auto self = shared_from_this();
    boost::asio::post(strand_, [this, self, msg = message.serialize()]() {
        std::lock_guard<std::mutex> lock(write_queue_mutex_);
        bool write_in_progress = !write_queue_.empty();
        write_queue_.push(std::move(msg));
        if (!write_in_progress) {
            doWrite();
        }
    });
}

bool AsioNetworkModel::doConnect(const boost::asio::ip::tcp::endpoint& endpoint) {
    auto self = shared_from_this();
    socket_.async_connect(endpoint,
                          boost::asio::bind_executor(strand_, [this, self](const boost::system::error_code& error) {
                              if (!error) {
                                  doRead();
                              }
                              else {
                                  // handleError(fmt::format("连接失败: {}", error.message()));
                                  handleError(fmt::format("连接失败: {}", error.message()));
                              }
                          }));
    return true;
}

void AsioNetworkModel::doRead() {
    auto self = shared_from_this();

    current_header_ = protocol::ProtocolHeader();
    // 1.读取固定长度的协议头
    boost::asio::async_read(
        socket_, boost::asio::buffer(&current_header_, sizeof(protocol::ProtocolHeader)),
        boost::asio::bind_executor(
            strand_, [this, self](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                if (error) {
                    handleError(fmt::format("读取协议头失败: {}", error.message()));
                    return;
                }

                if (!current_header_.validateSyncBytes()) {
                    handleError(fmt::format("协议头同步字节错误"));
                    return;
                }

                // 2. 读取消息体
                message_buffer_.resize(current_header_.length);
                memset(message_buffer_.data(), 0, current_header_.length);
                boost::asio::async_read(
                    socket_, boost::asio::buffer(message_buffer_),
                    boost::asio::bind_executor(strand_, [this, self](const boost::system::error_code& error,
                                                                     std::size_t /*bytes_transferred*/) {
                        if (error) {
                            handleError(fmt::format("读取消息体失败: {}", error.message()));
                            return;
                        }
                        // 3. 处理完整消息
                        processMessage(message_buffer_);

                        // 4. 继续读取下一条消息
                        doRead();
                    }));
            }));
}

void AsioNetworkModel::handleError(std::string_view error_msg) {
    spdlog::error("[{}]: 网络错误: {}, 请检查网络连接, 程序需要重新启动", common::getCurrentTimestamp(), error_msg);
    auto error_event = std::make_shared<common::NetworkErrorEvent>();
    error_event->message = std::string{error_msg};
    common::EventBus::getInstance().publish(error_event);
    disconnect();
}

void AsioNetworkModel::processMessage(const std::vector<std::uint8_t>& message_data) {
    try {
        std::string message{reinterpret_cast<const char*>(message_data.data()), message_data.size()};
        if (auto msg = protocol::MessageFactory::parseMessage(message)) {
            message_queue_.push(std::move(msg));
        }
        else {
            handleError(fmt::format("消息解析失败"));
        }
    }
    catch (const std::exception& e) {
        handleError(fmt::format("消息处理异常: {}", e.what()));
    }
}

void AsioNetworkModel::doWrite() {
    if (write_queue_.empty()) {
        return;
    }

    boost::asio::async_write(
        socket_, boost::asio::buffer(write_queue_.front()),
        boost::asio::bind_executor(
            strand_, [this, self = shared_from_this()](const boost::system::error_code& error,
                                                       std::size_t /*bytes_transferred*/) { handleWrite(error); }));
}

void AsioNetworkModel::handleWrite(const boost::system::error_code& error) {
    if (!error) {
        std::lock_guard<std::mutex> lock(write_queue_mutex_);
        write_queue_.pop();
        doWrite();
    }
    else {
        handleError(fmt::format("写入错误: {}", error.message()));
    }
}

}  // namespace network
