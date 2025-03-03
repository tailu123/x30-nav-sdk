#pragma once

#include <map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <condition_variable>
#include "../protocol/i_message.hpp"

namespace dog_navigation {
namespace network {

/**
 * @brief 请求来源类型
 */
enum class RequestSource {
    SYNC_REQUEST,    ///< 同步请求
    ASYNC_REQUEST    ///< 异步请求
};

/**
 * @brief 待处理请求信息
 */
struct PendingRequest {
    uint16_t sequenceNumber;                          ///< 序列号
    protocol::MessageType expectedResponseType;       ///< 预期响应类型
    std::chrono::steady_clock::time_point timestamp;  ///< 请求时间戳
    RequestSource source;                             ///< 请求来源
};

/**
 * @brief 请求管理器
 * 
 * 负责管理所有请求的生命周期，包括序列号生成、请求注册和响应匹配
 */
class RequestManager {
public:
    RequestManager() = default;
    ~RequestManager() = default;

    /**
     * @brief 生成唯一序列号
     * @return 唯一序列号
     */
    uint16_t generateSequenceNumber() {
        return sequenceCounter_++;
    }
    
    /**
     * @brief 添加待处理请求
     * @param seqNum 序列号
     * @param expectedType 预期响应类型
     * @param source 请求来源
     */
    void addPendingRequest(uint16_t seqNum, protocol::MessageType expectedType, 
                          RequestSource source) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        PendingRequest req;
        req.sequenceNumber = seqNum;
        req.expectedResponseType = expectedType;
        req.timestamp = std::chrono::steady_clock::now();
        req.source = source;
        pendingRequests_[seqNum] = req;
    }
    
    /**
     * @brief 移除待处理请求
     * @param seqNum 序列号
     * @return 是否成功移除
     */
    bool removePendingRequest(uint16_t seqNum) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        return pendingRequests_.erase(seqNum) > 0;
    }
    
    /**
     * @brief 检查是否有待处理请求
     * @param seqNum 序列号
     * @return 是否存在待处理请求
     */
    bool hasPendingRequest(uint16_t seqNum) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        return pendingRequests_.find(seqNum) != pendingRequests_.end();
    }
    
    /**
     * @brief 获取请求来源
     * @param seqNum 序列号
     * @return 请求来源，如果不存在则返回SYNC_REQUEST
     */
    RequestSource getRequestSource(uint16_t seqNum) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        auto it = pendingRequests_.find(seqNum);
        if (it != pendingRequests_.end()) {
            return it->second.source;
        }
        return RequestSource::SYNC_REQUEST; // 默认返回同步请求
    }
    
    /**
     * @brief 查找并移除匹配的请求
     * @param seqNum 序列号
     * @param type 响应类型
     * @param source 请求来源
     * @return 是否找到并移除
     */
    bool findAndRemoveMatchingRequest(uint16_t seqNum, protocol::MessageType type, 
                                     RequestSource source) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        auto it = pendingRequests_.find(seqNum);
        if (it != pendingRequests_.end() && 
            it->second.expectedResponseType == type &&
            it->second.source == source) {
            pendingRequests_.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * @brief 清理过期请求
     * @param timeout 超时时间
     */
    void cleanupExpiredRequests(std::chrono::milliseconds timeout) {
        std::lock_guard<std::mutex> lock(requestMutex_);
        auto now = std::chrono::steady_clock::now();
        for (auto it = pendingRequests_.begin(); it != pendingRequests_.end();) {
            if (now - it->second.timestamp > timeout) {
                it = pendingRequests_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    std::mutex requestMutex_;                         ///< 请求互斥锁
    std::atomic<uint16_t> sequenceCounter_{0};        ///< 序列号计数器
    std::map<uint16_t, PendingRequest> pendingRequests_; ///< 待处理请求映射
};

} // namespace network
} // namespace dog_navigation
