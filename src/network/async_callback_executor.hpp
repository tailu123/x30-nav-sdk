#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <iostream>

namespace dog_navigation {
namespace network {

/**
 * @brief 异步回调执行器
 * 
 * 负责在单独的线程中安全地执行回调函数，避免回调阻塞主线程或网络线程
 */
class AsyncCallbackExecutor {
public:
    /**
     * @brief 构造函数
     */
    AsyncCallbackExecutor() : running_(true) {
        workerThread_ = std::thread(&AsyncCallbackExecutor::workerThreadFunc, this);
    }
    
    /**
     * @brief 析构函数
     */
    ~AsyncCallbackExecutor() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            running_ = false;
        }
        queueCV_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
    
    /**
     * @brief 添加回调到队列
     * @tparam Callback 回调函数类型
     * @tparam Args 参数类型
     * @param callback 回调函数
     * @param args 参数
     */
    template<typename Callback, typename... Args>
    void enqueueCallback(Callback&& callback, Args&&... args) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        callbackQueue_.push([cb = std::forward<Callback>(callback), 
                            capturedArgs = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            try {
                std::apply(cb, std::move(capturedArgs));
            } catch (const std::exception& e) {
                // 记录异常但不传播
                std::cerr << "回调执行异常: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "回调执行未知异常" << std::endl;
            }
        });
        queueCV_.notify_one();
    }
    
    /**
     * @brief 获取队列中待处理的回调数量
     * @return 待处理回调数量
     */
    size_t getPendingCallbackCount() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return callbackQueue_.size();
    }
    
    /**
     * @brief 清空回调队列
     */
    void clearCallbacks() {
        std::lock_guard<std::mutex> lock(queueMutex_);
        std::queue<std::function<void()>> empty;
        std::swap(callbackQueue_, empty);
    }

private:
    /**
     * @brief 工作线程函数
     */
    void workerThreadFunc() {
        while (true) {
            std::function<void()> callback;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                queueCV_.wait(lock, [this] { 
                    return !running_ || !callbackQueue_.empty(); 
                });
                
                if (!running_ && callbackQueue_.empty()) {
                    break;
                }
                
                if (!callbackQueue_.empty()) {
                    callback = std::move(callbackQueue_.front());
                    callbackQueue_.pop();
                }
            }
            
            if (callback) {
                callback();
            }
        }
    }

private:
    std::thread workerThread_;                        ///< 工作线程
    std::queue<std::function<void()>> callbackQueue_; ///< 回调队列
    mutable std::mutex queueMutex_;                   ///< 队列互斥锁
    std::condition_variable queueCV_;                 ///< 条件变量
    std::atomic<bool> running_;                       ///< 运行标志
};

} // namespace network
} // namespace dog_navigation 