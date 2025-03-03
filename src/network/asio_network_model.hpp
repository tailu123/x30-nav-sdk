#pragma once

#include "base_network_model.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>

namespace network {

// 网络层回调接口
class INetworkCallback {
public:
    virtual ~INetworkCallback() = default;
    virtual void onMessageReceived(std::unique_ptr<protocol::IMessage> message) = 0;
};

/**
 * @brief 基于Boost.Asio的网络模型实现
 */
class AsioNetworkModel : public BaseNetworkModel {
public:
    /**
     * @brief 构造函数
     * @param callback 网络回调接口
     */
    explicit AsioNetworkModel(INetworkCallback& callback);

    /**
     * @brief 析构函数
     */
    ~AsioNetworkModel() override;

    /**
     * @brief 连接到服务器
     * @param host 主机地址
     * @param port 端口号
     * @return 是否连接成功
     */
    bool connect(const std::string& host, uint16_t port) override;

    /**
     * @brief 断开连接
     */
    void disconnect() override;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const override;

    /**
     * @brief 发送消息
     * @param message 要发送的消息
     * @return 是否发送成功
     */
    bool sendMessage(const protocol::IMessage& message) override;

private:
    /**
     * @brief 启动接收循环
     */
    void startReceive();

    /**
     * @brief 处理接收到的数据
     * @param error 错误码
     * @param bytes_transferred 接收到的字节数
     */
    void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * @brief 处理发送完成
     * @param error 错误码
     * @param bytes_transferred 发送的字节数
     */
    void handleSend(const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * @brief IO线程函数
     */
    void ioThreadFunc();

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context::strand strand_; // 用于序列化异步操作的执行器
    std::thread io_thread_;
    std::atomic<bool> connected_;
    // std::mutex mutex_;
    std::array<char, 4096> receive_buffer_;
    INetworkCallback& callback_;
    std::string receive_data_;
};

} // namespace network
