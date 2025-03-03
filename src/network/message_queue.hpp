#pragma once

#include "../protocol/i_message.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

namespace dog_navigation {
namespace network {

/**
 * @brief 消息队列接口
 */
class MessageQueue {
public:
    virtual ~MessageQueue() = default;

    /**
     * @brief 将消息推入队列
     * @param message 消息对象
     */
    virtual void pushMessage(std::unique_ptr<protocol::IMessage> message) = 0;

    /**
     * @brief 从队列中取出消息
     * @param message 用于存储取出的消息
     * @param timeout 超时时间
     * @return 是否成功取出消息
     */
    virtual bool popMessage(std::unique_ptr<protocol::IMessage>& message, 
                           std::chrono::milliseconds timeout) = 0;
};

/**
 * @brief 基本消息队列实现
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

/**
 * @brief 线程安全的消息队列，支持同步和异步消息分离
 * 
 * 此类维护两个独立的消息队列，一个用于同步请求的响应，一个用于异步请求的响应，
 * 避免同步和异步请求之间的相互干扰。
 */
class ThreadSafeMessageQueue {
public:
    /**
     * @brief 构造函数
     */
    ThreadSafeMessageQueue() = default;
    
    /**
     * @brief 析构函数
     */
    ~ThreadSafeMessageQueue() = default;
    
    /**
     * @brief 添加同步消息
     * @param message 消息对象
     */
    void pushSyncMessage(std::unique_ptr<protocol::IMessage> message) {
        if (!message) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(syncMutex_);
        syncQueue_.push(std::move(message));
        syncCV_.notify_one();
    }
    
    /**
     * @brief 添加异步消息
     * @param message 消息对象
     */
    void pushAsyncMessage(std::unique_ptr<protocol::IMessage> message) {
        if (!message) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(asyncMutex_);
        asyncQueue_.push(std::move(message));
        asyncCV_.notify_one();
    }
    
    /**
     * @brief 获取同步消息
     * @param message 用于存储取出的消息
     * @param timeout 超时时间
     * @return 是否成功取出消息
     */
    bool popSyncMessage(std::unique_ptr<protocol::IMessage>& message, 
                       std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(syncMutex_);
        if (syncCV_.wait_for(lock, timeout, [this] { return !syncQueue_.empty(); })) {
            message = std::move(syncQueue_.front());
            syncQueue_.pop();
            return true;
        }
        return false;
    }
    
    /**
     * @brief 获取异步消息
     * @param message 用于存储取出的消息
     * @param timeout 超时时间
     * @return 是否成功取出消息
     */
    bool popAsyncMessage(std::unique_ptr<protocol::IMessage>& message, 
                        std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(asyncMutex_);
        if (asyncCV_.wait_for(lock, timeout, [this] { return !asyncQueue_.empty(); })) {
            message = std::move(asyncQueue_.front());
            asyncQueue_.pop();
            return true;
        }
        return false;
    }
    
    /**
     * @brief 清空同步消息队列
     */
    void clearSyncQueue() {
        std::lock_guard<std::mutex> lock(syncMutex_);
        std::queue<std::unique_ptr<protocol::IMessage>> empty;
        std::swap(syncQueue_, empty);
    }
    
    /**
     * @brief 清空异步消息队列
     */
    void clearAsyncQueue() {
        std::lock_guard<std::mutex> lock(asyncMutex_);
        std::queue<std::unique_ptr<protocol::IMessage>> empty;
        std::swap(asyncQueue_, empty);
    }
    
    /**
     * @brief 获取同步队列中的消息数量
     * @return 消息数量
     */
    size_t getSyncQueueSize() const {
        std::lock_guard<std::mutex> lock(syncMutex_);
        return syncQueue_.size();
    }
    
    /**
     * @brief 获取异步队列中的消息数量
     * @return 消息数量
     */
    size_t getAsyncQueueSize() const {
        std::lock_guard<std::mutex> lock(asyncMutex_);
        return asyncQueue_.size();
    }

private:
    std::queue<std::unique_ptr<protocol::IMessage>> syncQueue_;  ///< 同步消息队列
    std::queue<std::unique_ptr<protocol::IMessage>> asyncQueue_; ///< 异步消息队列
    mutable std::mutex syncMutex_;                               ///< 同步队列互斥锁
    mutable std::mutex asyncMutex_;                              ///< 异步队列互斥锁
    std::condition_variable syncCV_;                             ///< 同步队列条件变量
    std::condition_variable asyncCV_;                            ///< 异步队列条件变量
};

} // namespace network
} // namespace dog_navigation
