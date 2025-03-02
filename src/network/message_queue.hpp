#pragma once

#include "asio_network_model.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>

namespace network {

/**
 * @brief 消息队列实现
 */
class MessageQueueImpl : public MessageQueue {
public:
    /**
     * @brief 构造函数
     */
    MessageQueueImpl() = default;

    /**
     * @brief 析构函数
     */
    ~MessageQueueImpl() override = default;

    /**
     * @brief 将消息推入队列
     * @param message 消息对象
     */
    void pushMessage(std::unique_ptr<protocol::IMessage> message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(message));
        cv_.notify_one();
    }

    /**
     * @brief 从队列中取出消息
     * @param message 用于存储取出的消息
     * @param timeout 超时时间
     * @return 是否成功取出消息
     */
    bool popMessage(std::unique_ptr<protocol::IMessage>& message, std::chrono::milliseconds timeout) override {
        std::unique_lock<std::mutex> lock(mutex_);

        // 等待队列非空或超时
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty(); })) {
            return false;
        }

        // 取出消息
        message = std::move(queue_.front());
        queue_.pop();

        return true;
    }

private:
    std::queue<std::unique_ptr<protocol::IMessage>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace network
