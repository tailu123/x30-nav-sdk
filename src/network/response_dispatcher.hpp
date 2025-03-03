#pragma once

#include <map>
#include <mutex>
#include <memory>
#include <chrono>
#include <functional>
#include "../protocol/i_message.hpp"
#include "request_manager.hpp"
#include "message_queue.hpp"

namespace dog_navigation {
namespace network {

/**
 * @brief 响应分发器
 * 
 * 负责处理接收到的响应并将其分发到正确的队列
 */
class ResponseDispatcher {
public:
    /**
     * @brief 构造函数
     * @param requestManager 请求管理器
     * @param messageQueue 消息队列
     */
    ResponseDispatcher(RequestManager& requestManager, ThreadSafeMessageQueue& messageQueue)
        : requestManager_(requestManager), messageQueue_(messageQueue) {}
    
    /**
     * @brief 处理接收到的响应
     * @param response 响应消息
     */
    void handleResponse(std::unique_ptr<protocol::IMessage> response) {
        if (!response) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(dispatchMutex_);
        uint16_t seqNum = response->getSequenceNumber();
        
        // 检查是否有等待此响应的请求
        if (requestManager_.hasPendingRequest(seqNum)) {
            // 根据请求来源分发响应
            RequestSource source = requestManager_.getRequestSource(seqNum);
            if (source == RequestSource::SYNC_REQUEST) {
                messageQueue_.pushSyncMessage(std::move(response));
            } else {
                messageQueue_.pushAsyncMessage(std::move(response));
            }
            
            // 请求已处理，从请求管理器中移除
            requestManager_.removePendingRequest(seqNum);
        } else {
            // 缓存未匹配的响应一段时间
            auto now = std::chrono::steady_clock::now();
            responseCache_[seqNum] = {std::move(response), now};
        }
    }
    
    /**
     * @brief 清理过期响应
     * @param maxAge 最大缓存时间
     */
    void cleanupExpiredResponses(std::chrono::milliseconds maxAge) {
        std::lock_guard<std::mutex> lock(dispatchMutex_);
        auto now = std::chrono::steady_clock::now();
        for (auto it = responseCache_.begin(); it != responseCache_.end();) {
            if (now - it->second.second > maxAge) {
                it = responseCache_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    /**
     * @brief 尝试匹配缓存的响应
     * 
     * 检查是否有新添加的请求可以匹配缓存中的响应
     */
    void tryMatchCachedResponses() {
        std::lock_guard<std::mutex> lock(dispatchMutex_);
        for (auto it = responseCache_.begin(); it != responseCache_.end();) {
            uint16_t seqNum = it->first;
            if (requestManager_.hasPendingRequest(seqNum)) {
                RequestSource source = requestManager_.getRequestSource(seqNum);
                if (source == RequestSource::SYNC_REQUEST) {
                    messageQueue_.pushSyncMessage(std::move(it->second.first));
                } else {
                    messageQueue_.pushAsyncMessage(std::move(it->second.first));
                }
                requestManager_.removePendingRequest(seqNum);
                it = responseCache_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    RequestManager& requestManager_;                  ///< 请求管理器引用
    ThreadSafeMessageQueue& messageQueue_;            ///< 消息队列引用
    std::mutex dispatchMutex_;                        ///< 分发互斥锁
    
    // 响应缓存：序列号 -> {响应, 时间戳}
    std::map<uint16_t, std::pair<std::unique_ptr<protocol::IMessage>, 
                                std::chrono::steady_clock::time_point>> responseCache_;
};

} // namespace network
} // namespace dog_navigation 