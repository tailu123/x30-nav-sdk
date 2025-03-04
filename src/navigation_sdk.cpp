#include <navigation_sdk.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <future>

// 包含网络和协议相关头文件
#include "network/asio_network_model.hpp"
// #include "network/message_queue.hpp"
#include "protocol/x30_protocol.hpp"


// using namespace network;
namespace nav_sdk {

// SDK版本
static const std::string SDK_VERSION = "0.1.0";

/**
 * @brief 安全回调包装函数，用于捕获和处理用户回调函数中可能抛出的异常
 * @tparam Callback 回调函数类型
 * @tparam Args 回调函数参数类型
 * @param callback 用户回调函数
 * @param callbackType 回调函数类型描述，用于日志记录
 * @param args 回调函数参数
 */
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    if (!callback) {
        return;
    }

    try {
        callback(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数异常: " << e.what() << std::endl;
    } catch (...) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数发生未知异常" << std::endl;
    }
}

// 事件转换为字符串的实现
std::string Event::toString() const {
    std::stringstream ss;

    // 格式化时间戳
    auto time_t_timestamp = std::chrono::system_clock::to_time_t(timestamp);
    ss << std::put_time(std::localtime(&time_t_timestamp), "%Y-%m-%d %H:%M:%S") << " - ";

    // 添加事件类型
    switch (type) {
        case EventType::CONNECTED:
            ss << "已连接";
            break;
        case EventType::DISCONNECTED:
            ss << "已断开连接";
            break;
        case EventType::NAVIGATION_STARTED:
            ss << "导航任务已开始";
            break;
        case EventType::NAVIGATION_COMPLETED:
            ss << "导航任务已完成";
            break;
        case EventType::NAVIGATION_FAILED:
            ss << "导航任务失败";
            break;
        case EventType::STATUS_UPDATED:
            ss << "状态已更新";
            break;
        case EventType::ERROR_OCCURRED:
            ss << "发生错误";
            break;
    }

    // 添加消息
    if (!message.empty()) {
        ss << ": " << message;
    }

    return ss.str();
}

// protocol::GetRealTimeStatusResponse -> RealTimeStatus
RealTimeStatus convertToRealTimeStatus(const protocol::GetRealTimeStatusResponse& realTimeResp) {
    RealTimeStatus status;
    status.motionState = realTimeResp.motionState;
    status.posX = realTimeResp.posX;
    status.posY = realTimeResp.posY;
    status.posZ = realTimeResp.posZ;
    status.angleYaw = realTimeResp.angleYaw;
    status.roll = realTimeResp.roll;
    status.pitch = realTimeResp.pitch;
    status.yaw = realTimeResp.yaw;
    status.speed = realTimeResp.speed;
    status.curOdom = realTimeResp.curOdom;
    status.sumOdom = realTimeResp.sumOdom;
    status.curRuntime = realTimeResp.curRuntime;
    status.sumRuntime = realTimeResp.sumRuntime;
    status.res = realTimeResp.res;
    status.x0 = realTimeResp.x0;
    status.y0 = realTimeResp.y0;
    status.h = realTimeResp.h;
    status.electricity = realTimeResp.electricity;
    status.location = realTimeResp.location;
    status.RTKState = realTimeResp.RTKState;
    status.onDockState = realTimeResp.onDockState;
    status.gaitState = realTimeResp.gaitState;
    status.motorState = realTimeResp.motorState;
    status.chargeState = realTimeResp.chargeState;
    status.controlMode = realTimeResp.controlMode;
    status.mapUpdateState = realTimeResp.mapUpdateState;
    status.timestamp = realTimeResp.timestamp;

    return status;
}

// SDK实现类
class NavigationSdkImpl : public ::network::INetworkCallback {
public:
    NavigationSdkImpl(const SdkOptions& options)
        : options_(options),
          connected_(false),
          network_model_(std::make_unique<::network::AsioNetworkModel>(*this)) {
    }

    ~NavigationSdkImpl() {
        disconnect();
    }

    bool connect(const std::string& host, uint16_t port) {
        if (connected_) {
            return true;
        }

        if (network_model_->connect(host, port)) {
            connected_ = true;

            // 触发连接事件
            if (event_callback_) {
                Event event;
                event.type = EventType::CONNECTED;
                event.message = "已连接到 " + host + ":" + std::to_string(port);
                event.timestamp = std::chrono::system_clock::now();
                safeCallback(event_callback_, "事件", event);
            }

            return true;
        }

        return false;
    }

    void disconnect() {
        if (!connected_) {
            return;
        }

        network_model_->disconnect();
        connected_ = false;

        // 触发断开连接事件
        if (event_callback_) {
            Event event;
            event.type = EventType::DISCONNECTED;
            event.message = "已断开连接";
            event.timestamp = std::chrono::system_clock::now();
            safeCallback(event_callback_, "事件", event);
        }
    }

    bool isConnected() const {
        return connected_ && network_model_->isConnected();
    }

    void setEventCallback(EventCallback callback) {
        event_callback_ = std::move(callback);
    }

    RealTimeStatus getRealTimeStatus() {
        try {
            if (!isConnected()) {
                throw std::runtime_error("未连接到服务器");
            }

            // 创建请求消息
            protocol::GetRealTimeStatusRequest request;
            request.timestamp = getCurrentTimestamp();

            // 生成并设置序列号
            uint16_t seqNum = generateSequenceNumber();
            request.setSequenceNumber(seqNum);

            // 添加到待处理请求，标记为同步请求
            addPendingRequest(seqNum, protocol::MessageType::GET_REAL_TIME_STATUS_RESP);

            // 发送请求
            network_model_->sendMessage(request);

            // 等待响应
            std::unique_ptr<protocol::GetRealTimeStatusResponse> realTimeResp;
            {
                std::unique_lock<std::mutex> lock(pending_requests_mutex_);
                auto& pendingReq = pendingRequests_[seqNum];

                if (!pendingReq.responseReceived) {
                    pendingReq.cv->wait_for(lock, options_.requestTimeout,
                        [&pendingReq]() { return pendingReq.responseReceived; });
                }

                if (pendingReq.responseReceived && pendingReq.response) {
                    // 正确处理指针转换和所有权转移
                    auto* castedPtr = dynamic_cast<protocol::GetRealTimeStatusResponse*>(pendingReq.response.get());
                    if (castedPtr) {
                        // 创建新的 unique_ptr 并释放原始 response 的所有权
                        realTimeResp = std::unique_ptr<protocol::GetRealTimeStatusResponse>(castedPtr);
                        pendingReq.response.release();  // 释放所有权但不删除对象
                    } else {
                        throw std::runtime_error("dynamic_cast 转换失败");
                    }
                }

                // 移除请求
                pendingRequests_.erase(seqNum);
            }

            if (!realTimeResp) {
                throw std::runtime_error("收到无效的实时状态响应");
            }

            // 转换为SDK的RealTimeStatus
            RealTimeStatus status = convertToRealTimeStatus(*realTimeResp);

            return status;
        } catch (const std::exception& e) {
            std::cerr << "getRealTimeStatus 异常: " << e.what() << std::endl;
            throw;
        } catch (...) {
            std::cerr << "getRealTimeStatus 未知异常" << std::endl;
            throw;
        }
    }

    // 添加基于回调的异步方法实现
    void startNavigationAsync(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
        if (!callback) {
            return;
        }

        if (!isConnected()) {
            NavigationResult failResult;
            failResult.errorCode = ErrorCode::NOT_CONNECTED;
            failResult.timestamp = getCurrentTimestamp();
            safeCallback(callback, "导航结果", failResult);
            return;
        }

        if (points.empty()) {
            NavigationResult failResult;
            failResult.errorCode = ErrorCode::INVALID_PARAM;
            failResult.timestamp = getCurrentTimestamp();
            safeCallback(callback, "导航结果", failResult);
            return;
        }

        // 创建请求消息
        protocol::NavigationTaskRequest request;
        request.timestamp = getCurrentTimestamp();

        // 生成并设置序列号
        uint16_t seqNum = generateSequenceNumber();
        request.setSequenceNumber(seqNum);

        // 转换导航点
        for (const auto& point : points) {
            protocol::NavigationPoint proto_point;
            proto_point.mapId = point.mapId;
            proto_point.value = point.value;
            proto_point.posX = point.posX;
            proto_point.posY = point.posY;
            proto_point.posZ = point.posZ;
            proto_point.angleYaw = point.angleYaw;
            proto_point.pointInfo = point.pointInfo;
            proto_point.gait = point.gait;
            proto_point.speed = point.speed;
            proto_point.manner = point.manner;
            proto_point.obsMode = point.obsMode;
            proto_point.navMode = point.navMode;
            proto_point.terrain = point.terrain;
            proto_point.posture = point.posture;
            request.points.push_back(proto_point);
        }

        // 保存回调函数
        {
            std::lock_guard<std::mutex> lock(navigation_result_callbacks_mutex_);
            navigation_result_callbacks_[seqNum] = std::move(callback);
        }

        // 发送请求
        network_model_->sendMessage(request);
    }

    bool cancelNavigation() {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        // 创建请求消息
        protocol::CancelTaskRequest request;
        request.timestamp = getCurrentTimestamp();

        // 生成并设置序列号
        uint16_t seqNum = generateSequenceNumber();
        request.setSequenceNumber(seqNum);

        // 添加到待处理请求，标记为同步请求
        addPendingRequest(seqNum, protocol::MessageType::CANCEL_TASK_RESP);

        // 发送请求
        network_model_->sendMessage(request);

        // 等待响应
        std::unique_ptr<protocol::CancelTaskResponse> cancelResp;
        {
            std::unique_lock<std::mutex> lock(pending_requests_mutex_);
            auto& pendingReq = pendingRequests_[seqNum];

            if (!pendingReq.responseReceived) {
                pendingReq.cv->wait_for(lock, options_.requestTimeout,
                    [&pendingReq]() { return pendingReq.responseReceived; });
            }

            if (pendingReq.responseReceived && pendingReq.response) {
                // 正确处理指针转换和所有权转移
                auto* castedPtr = dynamic_cast<protocol::CancelTaskResponse*>(pendingReq.response.get());
                if (castedPtr) {
                    cancelResp = std::unique_ptr<protocol::CancelTaskResponse>(castedPtr);
                    pendingReq.response.release();  // 释放所有权但不删除对象
                } else {
                    throw std::runtime_error("dynamic_cast 转换失败");
                }
            }

            pendingRequests_.erase(seqNum);
        }

        if (!cancelResp) {
            throw std::runtime_error("收到无效的取消任务响应");
        }

        return cancelResp->errorCode == protocol::ErrorCode::SUCCESS;
    }

    TaskStatusResult queryTaskStatus() {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        // 创建请求消息
        protocol::QueryStatusRequest request;
        request.timestamp = getCurrentTimestamp();

        // 生成并设置序列号
        uint16_t seqNum = generateSequenceNumber();
        request.setSequenceNumber(seqNum);

        // 添加到待处理请求，标记为同步请求
        addPendingRequest(seqNum, protocol::MessageType::QUERY_STATUS_RESP);

        // 发送请求
        network_model_->sendMessage(request);

        // 等待响应
        std::unique_ptr<protocol::QueryStatusResponse> statusResp;
        {
            std::unique_lock<std::mutex> lock(pending_requests_mutex_);
            auto& pendingReq = pendingRequests_[seqNum];

            if (!pendingReq.responseReceived) {
                pendingReq.cv->wait_for(lock, options_.requestTimeout,
                    [&pendingReq]() { return pendingReq.responseReceived; });
            }

            if (pendingReq.responseReceived && pendingReq.response) {
                // 正确处理指针转换和所有权转移
                auto* castedPtr = dynamic_cast<protocol::QueryStatusResponse*>(pendingReq.response.get());
                if (castedPtr) {
                    statusResp = std::unique_ptr<protocol::QueryStatusResponse>(castedPtr);
                    pendingReq.response.release();  // 释放所有权但不删除对象
                } else {
                    throw std::runtime_error("dynamic_cast 转换失败");
                }
            }

            pendingRequests_.erase(seqNum);
        }

        if (!statusResp) {
            throw std::runtime_error("收到无效的任务状态响应");
        }

        // 转换为SDK的TaskStatusResult
        TaskStatusResult result;
        result.timestamp = statusResp->timestamp;
        result.status = static_cast<NavigationStatus>(statusResp->status);
        result.errorCode = static_cast<ErrorCode>(statusResp->errorCode);
        result.value = statusResp->value;

        return result;
    }

    // 实现网络回调接口
    void onMessageReceived(std::unique_ptr<protocol::IMessage> message) override {
        if (!message) {
            return;
        }

        uint16_t seqNum = message->getSequenceNumber();
        protocol::MessageType msgType = message->getType();

        if (msgType == protocol::MessageType::NAVIGATION_TASK_RESP) {

            NavigationResultCallback callback;
            NavigationResult result;
            // 检查是否有等待此响应的请求
            {
                std::lock_guard<std::mutex> lock(navigation_result_callbacks_mutex_);
                auto it = navigation_result_callbacks_.find(seqNum);
                if (it != navigation_result_callbacks_.end()) {
                    callback = it->second;
                    navigation_result_callbacks_.erase(it);
                }
            }

            // 如果有回调，则使用安全回调包装函数调用
            if (callback) {
                auto resp = static_cast<protocol::NavigationTaskResponse*>(message.get());
                result.value = resp->value;
                result.errorCode = static_cast<ErrorCode>(resp->errorCode);
                result.errorStatus = resp->errorStatus;
                result.timestamp = resp->timestamp;
                safeCallback(callback, "导航结果", result);
            }

            return;
        }

        // 处理其他类型的响应消息
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        auto it = pendingRequests_.find(seqNum);
        if (it != pendingRequests_.end() && it->second.expectedResponseType == msgType) {
            it->second.response = std::move(message);
            it->second.responseReceived = true;
            it->second.cv->notify_one();
        }
    }

private:
    bool isRequestPending(uint16_t sequenceNumber) {
        return pendingRequests_.find(sequenceNumber) != pendingRequests_.end();
    }

    void addPendingRequest(uint16_t sequenceNumber, protocol::MessageType expectedType) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        PendingRequest req;
        req.expectedResponseType = expectedType;
        req.responseReceived = false;
        req.cv = std::make_shared<std::condition_variable>();
        pendingRequests_.emplace(sequenceNumber, std::move(req));
    }

    void removePendingRequest(uint16_t sequenceNumber) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pendingRequests_.erase(sequenceNumber);
    }

    // 获取当前时间戳
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    SdkOptions options_;
    std::atomic<bool> connected_;
    EventCallback event_callback_;
    std::unique_ptr<::network::AsioNetworkModel> network_model_;

    // 生成序列号， 从0到65535后溢出回到0
    uint16_t generateSequenceNumber() {
        static std::atomic<uint16_t> sequenceNumber = 0;
        return ++sequenceNumber;
    }

    struct PendingRequest {
        protocol::MessageType expectedResponseType{};
        std::unique_ptr<protocol::IMessage> response{};
        bool responseReceived{false};
        std::shared_ptr<std::condition_variable> cv;
    };

    std::mutex pending_requests_mutex_;  // 保护 pendingRequests_ 的互斥锁
    std::map<uint16_t, PendingRequest> pendingRequests_;

    // TODO: 没有超时清理
    std::mutex navigation_result_callbacks_mutex_;
    std::map<uint16_t, NavigationResultCallback> navigation_result_callbacks_;
};

// NavigationSdk类的实现
NavigationSdk::NavigationSdk(const SdkOptions& options)
    : impl_(std::make_unique<NavigationSdkImpl>(options)) {
}

NavigationSdk::~NavigationSdk() = default;

bool NavigationSdk::connect(const std::string& host, uint16_t port) {
    return impl_->connect(host, port);
}

void NavigationSdk::disconnect() {
    impl_->disconnect();
}

bool NavigationSdk::isConnected() const {
    return impl_->isConnected();
}

void NavigationSdk::setEventCallback(EventCallback callback) {
    impl_->setEventCallback(std::move(callback));
}

RealTimeStatus NavigationSdk::getRealTimeStatus() {
    return impl_->getRealTimeStatus();
}

// 添加基于回调的异步方法实现
void NavigationSdk::startNavigationAsync(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
    impl_->startNavigationAsync(points, std::move(callback));
}

bool NavigationSdk::cancelNavigation() {
    return impl_->cancelNavigation();
}

TaskStatusResult NavigationSdk::queryTaskStatus() {
    return impl_->queryTaskStatus();
}

std::string NavigationSdk::getVersion() {
    return SDK_VERSION;
}

} // namespace nav_sdk
