#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include "../protocol/i_message.hpp"
#include "message_queue.hpp"
#include "request_manager.hpp"
#include "response_dispatcher.hpp"
#include "async_callback_executor.hpp"

namespace dog_navigation {
namespace network {

/**
 * @brief 增强的网络模型
 * 
 * 整合请求管理器、响应分发器和线程安全消息队列，提高 SDK 的健壮性
 */
class EnhancedNetworkModel {
public:
    /**
     * @brief 构造函数
     */
    EnhancedNetworkModel() 
        : running_(false), 
          requestManager_(),
          messageQueue_(),
          responseDispatcher_(requestManager_, messageQueue_),
          callbackExecutor_() {
    }
    
    /**
     * @brief 析构函数
     */
    ~EnhancedNetworkModel() {
        stop();
    }
    
    /**
     * @brief 启动网络模型
     * @return 是否成功启动
     */
    bool start() {
        if (running_) {
            return true;
        }
        
        running_ = true;
        
        // 启动清理线程
        cleanupThread_ = std::thread(&EnhancedNetworkModel::cleanupThreadFunc, this);
        
        return true;
    }
    
    /**
     * @brief 停止网络模型
     */
    void stop() {
        if (!running_) {
            return;
        }
        
        running_ = false;
        
        // 等待清理线程结束
        if (cleanupThread_.joinable()) {
            cleanupThread_.join();
        }
        
        // 清空消息队列
        messageQueue_.clearSyncQueue();
        messageQueue_.clearAsyncQueue();
    }
    
    /**
     * @brief 发送同步请求
     * @tparam ResponseType 响应类型
     * @param request 请求消息
     * @param expectedResponseType 预期响应类型
     * @param timeout 超时时间
     * @return 响应消息，如果超时或失败则返回nullptr
     */
    template<typename ResponseType>
    std::unique_ptr<ResponseType> sendSyncRequest(
        const protocol::IMessage& request,
        protocol::MessageType expectedResponseType,
        std::chrono::milliseconds timeout) {
        
        // 生成序列号
        uint16_t seqNum = requestManager_.generateSequenceNumber();
        const_cast<protocol::IMessage&>(request).setSequenceNumber(seqNum);
        
        // 注册请求
        requestManager_.addPendingRequest(seqNum, expectedResponseType, RequestSource::SYNC_REQUEST);
        
        // 发送请求
        if (!sendMessage(request)) {
            requestManager_.removePendingRequest(seqNum);
            return nullptr;
        }
        
        // 等待响应
        std::unique_ptr<protocol::IMessage> response;
        bool received = messageQueue_.popSyncMessage(response, timeout);
        
        if (!received || !response) {
            requestManager_.removePendingRequest(seqNum);
            return nullptr;
        }
        
        // 验证响应类型和序列号
        if (response->getType() != expectedResponseType || 
            response->getSequenceNumber() != seqNum) {
            return nullptr;
        }
        
        // 转换为具体类型
        return std::unique_ptr<ResponseType>(dynamic_cast<ResponseType*>(response.release()));
    }
    
    /**
     * @brief 发送异步请求
     * @tparam ResponseType 响应类型
     * @tparam Callback 回调函数类型
     * @param request 请求消息
     * @param expectedResponseType 预期响应类型
     * @param timeout 超时时间
     * @param callback 回调函数
     */
    template<typename ResponseType, typename Callback>
    void sendAsyncRequest(
        const protocol::IMessage& request,
        protocol::MessageType expectedResponseType,
        std::chrono::milliseconds timeout,
        Callback&& callback) {
        
        // 生成序列号
        uint16_t seqNum = requestManager_.generateSequenceNumber();
        const_cast<protocol::IMessage&>(request).setSequenceNumber(seqNum);
        
        // 注册请求
        requestManager_.addPendingRequest(seqNum, expectedResponseType, RequestSource::ASYNC_REQUEST);
        
        // 发送请求
        if (!sendMessage(request)) {
            requestManager_.removePendingRequest(seqNum);
            callbackExecutor_.enqueueCallback(std::forward<Callback>(callback), nullptr);
            return;
        }
        
        // 在单独的线程中等待响应
        std::thread([this, seqNum, expectedResponseType, timeout, callback = std::forward<Callback>(callback)]() {
            // 等待响应
            std::unique_ptr<protocol::IMessage> response;
            bool received = messageQueue_.popAsyncMessage(response, timeout);
            
            if (!received || !response) {
                requestManager_.removePendingRequest(seqNum);
                callbackExecutor_.enqueueCallback(callback, nullptr);
                return;
            }
            
            // 验证响应类型和序列号
            if (response->getType() != expectedResponseType || 
                response->getSequenceNumber() != seqNum) {
                callbackExecutor_.enqueueCallback(callback, nullptr);
                return;
            }
            
            // 转换为具体类型
            auto typedResponse = std::unique_ptr<ResponseType>(
                dynamic_cast<ResponseType*>(response.release()));
            
            // 通过回调执行器安全地执行回调
            callbackExecutor_.enqueueCallback(callback, std::move(typedResponse));
        }).detach();
    }
    
    /**
     * @brief 处理接收到的消息
     * @param message 接收到的消息
     */
    void handleReceivedMessage(std::unique_ptr<protocol::IMessage> message) {
        if (!message) {
            return;
        }
        
        // 使用响应分发器处理消息
        responseDispatcher_.handleResponse(std::move(message));
    }
    
    /**
     * @brief 发送消息
     * @param message 要发送的消息
     * @return 是否成功发送
     */
    bool sendMessage(const protocol::IMessage& message) {
        // 实际发送消息的实现
        // 这里需要根据具体的网络实现来发送消息
        // 例如：network_model_->sendMessage(message);
        return true;
    }

private:
    /**
     * @brief 清理线程函数
     * 
     * 定期清理过期的请求和响应
     */
    void cleanupThreadFunc() {
        const auto cleanupInterval = std::chrono::seconds(30);
        const auto maxAge = std::chrono::minutes(5);
        
        while (running_) {
            // 清理过期请求
            requestManager_.cleanupExpiredRequests(maxAge);
            
            // 清理过期响应
            responseDispatcher_.cleanupExpiredResponses(maxAge);
            
            // 尝试匹配缓存的响应
            responseDispatcher_.tryMatchCachedResponses();
            
            // 等待下一次清理
            std::this_thread::sleep_for(cleanupInterval);
        }
    }

private:
    std::atomic<bool> running_;                      ///< 运行标志
    RequestManager requestManager_;                  ///< 请求管理器
    ThreadSafeMessageQueue messageQueue_;            ///< 消息队列
    ResponseDispatcher responseDispatcher_;          ///< 响应分发器
    AsyncCallbackExecutor callbackExecutor_;         ///< 回调执行器
    std::thread cleanupThread_;                      ///< 清理线程
};

} // namespace network
} // namespace dog_navigation 