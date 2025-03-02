#pragma once

#include <sys/epoll.h>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "network/base_network_model.hpp"

namespace common {
class MessageQueue;
}

namespace network {

class EpollNetworkModel : public BaseNetworkModel {
public:
    explicit EpollNetworkModel(common::MessageQueue& message_queue);
    ~EpollNetworkModel() override;

    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool isConnected() const override;
    void sendMessage(const protocol::IMessage& message) override;

private:
    // 初始化epoll
    bool initEpoll();
    // 事件循环线程函数
    void eventLoop();
    // 处理网络事件
    void poll();
    // 处理读事件
    bool handleRead();
    // 处理写事件
    bool handleWrite();
    // 设置非阻塞
    void setNonBlocking(int fd);

private:
    void handleError(std::string_view error_msg);

    std::thread event_thread_;
    std::atomic<bool> running_;

    common::MessageQueue& message_queue_;
    int epoll_fd_;
    int socket_fd_;
    std::atomic<bool> connected_;

    std::queue<std::string> write_queue_;
    std::mutex write_mutex_;
};

}  // namespace network
