#pragma once

#include <boost/asio.hpp>
#include <queue>
#include "network/base_network_model.hpp"
#include "protocol/protocol_header.hpp"

namespace common {
class MessageQueue;
}  // namespace common

namespace network {
class AsioNetworkModel : public BaseNetworkModel, public std::enable_shared_from_this<AsioNetworkModel> {
public:
    explicit AsioNetworkModel(common::MessageQueue& message_queue);
    ~AsioNetworkModel() override;

    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool isConnected() const override;

    void sendMessage(const protocol::IMessage& message) override;

private:
    bool doConnect(const boost::asio::ip::tcp::endpoint& endpoint);
    void doRead();
    void doWrite();
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);
    void handleWrite(const boost::system::error_code& error);
    void processMessage(const std::vector<std::uint8_t>& message_data);
    void handleError(std::string_view error_msg);

    // ASIO相关成员
    boost::asio::io_context io_context_;
    std::thread io_thread_;
    boost::asio::io_context::work work_;

    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    common::MessageQueue& message_queue_;
    boost::asio::streambuf read_buffer_;
    std::queue<std::string> write_queue_;
    std::mutex write_queue_mutex_;
    protocol::ProtocolHeader current_header_;
    std::vector<std::uint8_t> message_buffer_;
};
}  // namespace network
