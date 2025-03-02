#include <dog_navigation/navigation_sdk.h>
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
#include "network/message_queue.hpp"
#include "protocol/x30_protocol.hpp"

namespace dog_navigation {

// SDK版本
static const std::string SDK_VERSION = "0.1.0";

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

// SDK实现类
class NavigationSdkImpl : public network::MessageQueue {
public:
    NavigationSdkImpl(const SdkOptions& options)
        : options_(options),
          connected_(false),
          network_model_(std::make_unique<network::AsioNetworkModel>(*this)) {
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
                event_callback_(event);
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
            event_callback_(event);
        }
    }

    bool isConnected() const {
        return connected_ && network_model_->isConnected();
    }

    void setEventCallback(EventCallback callback) {
        event_callback_ = std::move(callback);
    }

    RealTimeStatus getRealTimeStatus() {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        // 创建请求消息
        protocol::GetRealTimeStatusRequest request;
        request.timestamp = getCurrentTimestamp();

        // 发送请求
        network_model_->sendMessage(request);

        // 等待响应
        std::unique_ptr<protocol::IMessage> response;
        if (waitForResponse(response, options_.requestTimeout)) {
            if (response->getType() == protocol::MessageType::GET_REAL_TIME_STATUS_RESP) {
                auto* status_resp = static_cast<protocol::GetRealTimeStatusResponse*>(response.get());

                // 转换为SDK的RealTimeStatus
                RealTimeStatus result;
                result.motionState = status_resp->motionState;
                result.posX = status_resp->posX;
                result.posY = status_resp->posY;
                result.posZ = status_resp->posZ;
                result.angleYaw = status_resp->angleYaw;
                result.roll = status_resp->roll;
                result.pitch = status_resp->pitch;
                result.yaw = status_resp->yaw;
                result.speed = status_resp->speed;
                result.curOdom = status_resp->curOdom;
                result.sumOdom = status_resp->sumOdom;
                result.curRuntime = status_resp->curRuntime;
                result.sumRuntime = status_resp->sumRuntime;
                result.res = status_resp->res;
                result.x0 = status_resp->x0;
                result.y0 = status_resp->y0;
                result.h = status_resp->h;
                result.electricity = status_resp->electricity;
                result.location = status_resp->location;
                result.RTKState = status_resp->RTKState;
                result.onDockState = status_resp->onDockState;
                result.gaitState = status_resp->gaitState;
                result.motorState = status_resp->motorState;
                result.chargeState = status_resp->chargeState;
                result.controlMode = status_resp->controlMode;
                result.mapUpdateState = status_resp->mapUpdateState;
                result.timestamp = status_resp->timestamp;

                return result;
            }
        }

        throw std::runtime_error("获取实时状态失败");
    }

    // 恢复原有的基于 future 的异步方法
    std::future<RealTimeStatus> getRealTimeStatusAsync() {
        return std::async(std::launch::async, [this]() {
            return getRealTimeStatus();
        });
    }

    // 添加基于回调的异步方法实现
    void getRealTimeStatusAsync(RealTimeStatusCallback callback) {
        if (!callback) {
            return;
        }

        std::thread([this, callback = std::move(callback)]() {
            try {
                RealTimeStatus status = getRealTimeStatus();
                callback(status, ErrorCode::SUCCESS);
            } catch (const std::exception& e) {
                // 创建一个空的状态对象
                RealTimeStatus emptyStatus;
                emptyStatus.timestamp = getCurrentTimestamp();
                callback(emptyStatus, ErrorCode::FAILURE);
            }
        }).detach();
    }

    NavigationResult startNavigation(const std::vector<NavigationPoint>& points) {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        if (points.empty()) {
            throw std::invalid_argument("导航点列表不能为空");
        }

        // 创建请求消息
        protocol::NavigationTaskRequest request;
        request.timestamp = getCurrentTimestamp();

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

        // 发送请求
        network_model_->sendMessage(request);

        // 触发导航开始事件
        if (event_callback_) {
            Event event;
            event.type = EventType::NAVIGATION_STARTED;
            event.message = "导航任务已开始，共 " + std::to_string(points.size()) + " 个导航点";
            event.timestamp = std::chrono::system_clock::now();
            event_callback_(event);
        }

        // 等待响应
        std::unique_ptr<protocol::IMessage> response;
        if (waitForResponse(response, options_.requestTimeout)) {
            if (response->getType() == protocol::MessageType::NAVIGATION_TASK_RESP) {
                auto* nav_resp = static_cast<protocol::NavigationTaskResponse*>(response.get());

                // 转换为SDK的NavigationResult
                NavigationResult result;
                result.value = nav_resp->value;

                switch (nav_resp->errorCode) {
                    case protocol::ErrorCode::SUCCESS:
                        result.errorCode = ErrorCode::SUCCESS;
                        break;
                    case protocol::ErrorCode::FAILURE:
                        result.errorCode = ErrorCode::FAILURE;
                        break;
                    case protocol::ErrorCode::CANCELLED:
                        result.errorCode = ErrorCode::CANCELLED;
                        break;
                    default:
                        result.errorCode = ErrorCode::FAILURE;
                        break;
                }

                result.errorStatus = nav_resp->errorStatus;
                result.timestamp = nav_resp->timestamp;

                // 触发相应事件
                if (event_callback_) {
                    Event event;
                    if (result.errorCode == ErrorCode::SUCCESS) {
                        event.type = EventType::NAVIGATION_STARTED;
                        event.message = "导航任务已接受，目标点编号: " + std::to_string(result.value);
                    } else {
                        event.type = EventType::NAVIGATION_FAILED;
                        event.message = "导航任务失败，错误码: " + std::to_string(static_cast<int>(result.errorCode));
                    }
                    event.timestamp = std::chrono::system_clock::now();
                    event_callback_(event);
                }

                return result;
            }
        }

        // 如果没有收到响应，返回失败结果
        NavigationResult result;
        result.errorCode = ErrorCode::TIMEOUT;
        result.timestamp = getCurrentTimestamp();

        return result;
    }

    // 恢复原有的基于 future 的异步方法
    std::future<NavigationResult> startNavigationAsync(const std::vector<NavigationPoint>& points) {
        return std::async(std::launch::async, [this, points]() {
            return startNavigation(points);
        });
    }

    // 添加基于回调的异步方法实现
    void startNavigationAsync(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
        if (!callback) {
            return;
        }

        std::thread([this, points, callback = std::move(callback)]() {
            try {
                NavigationResult result = startNavigation(points);
                callback(result);
            } catch (const std::exception& e) {
                // 创建一个错误结果
                NavigationResult errorResult;
                errorResult.errorCode = ErrorCode::FAILURE;
                errorResult.timestamp = getCurrentTimestamp();
                callback(errorResult);
            }
        }).detach();
    }

    bool cancelNavigation() {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        // 创建请求消息
        protocol::CancelTaskRequest request;
        request.timestamp = getCurrentTimestamp();

        // 发送请求
        network_model_->sendMessage(request);

        // 等待响应
        std::unique_ptr<protocol::IMessage> response;
        if (waitForResponse(response, options_.requestTimeout)) {
            if (response->getType() == protocol::MessageType::CANCEL_TASK_RESP) {
                auto* cancel_resp = static_cast<protocol::CancelTaskResponse*>(response.get());

                return cancel_resp->errorCode == protocol::ErrorCode::SUCCESS;
            }
        }

        return false;
    }

    // 恢复原有的基于 future 的异步方法
    std::future<bool> cancelNavigationAsync() {
        return std::async(std::launch::async, [this]() {
            return cancelNavigation();
        });
    }

    // 添加基于回调的异步方法实现
    void cancelNavigationAsync(OperationResultCallback callback) {
        if (!callback) {
            return;
        }

        std::thread([this, callback = std::move(callback)]() {
            try {
                bool result = cancelNavigation();
                callback(result, result ? ErrorCode::SUCCESS : ErrorCode::FAILURE);
            } catch (const std::exception& e) {
                callback(false, ErrorCode::FAILURE);
            }
        }).detach();
    }

    TaskStatusResult queryTaskStatus() {
        if (!isConnected()) {
            throw std::runtime_error("未连接到服务器");
        }

        // 创建请求消息
        protocol::QueryStatusRequest request;
        request.timestamp = getCurrentTimestamp();

        // 发送请求
        network_model_->sendMessage(request);

        // 等待响应
        std::unique_ptr<protocol::IMessage> response;
        if (waitForResponse(response, options_.requestTimeout)) {
            if (response->getType() == protocol::MessageType::QUERY_STATUS_RESP) {
                auto* status_resp = static_cast<protocol::QueryStatusResponse*>(response.get());

                // 转换为SDK的TaskStatusResult
                TaskStatusResult result;
                result.value = status_resp->value;

                switch (status_resp->status) {
                    case protocol::NavigationStatus::COMPLETED:
                        result.status = NavigationStatus::COMPLETED;
                        break;
                    case protocol::NavigationStatus::EXECUTING:
                        result.status = NavigationStatus::EXECUTING;
                        break;
                    case protocol::NavigationStatus::FAILED:
                        result.status = NavigationStatus::FAILED;
                        break;
                    default:
                        result.status = NavigationStatus::FAILED;
                        break;
                }

                switch (status_resp->errorCode) {
                    case protocol::ErrorCode::SUCCESS:
                        result.errorCode = ErrorCode::SUCCESS;
                        break;
                    case protocol::ErrorCode::FAILURE:
                        result.errorCode = ErrorCode::FAILURE;
                        break;
                    case protocol::ErrorCode::CANCELLED:
                        result.errorCode = ErrorCode::CANCELLED;
                        break;
                    default:
                        result.errorCode = ErrorCode::FAILURE;
                        break;
                }

                result.timestamp = status_resp->timestamp;

                // 如果任务已完成或失败，触发相应事件
                if (event_callback_ &&
                    (result.status == NavigationStatus::COMPLETED ||
                     result.status == NavigationStatus::FAILED)) {
                    Event event;
                    if (result.status == NavigationStatus::COMPLETED) {
                        event.type = EventType::NAVIGATION_COMPLETED;
                        event.message = "导航任务已完成，目标点编号: " + std::to_string(result.value);
                    } else {
                        event.type = EventType::NAVIGATION_FAILED;
                        event.message = "导航任务失败，目标点编号: " + std::to_string(result.value);
                    }
                    event.timestamp = std::chrono::system_clock::now();
                    event_callback_(event);
                }

                return result;
            }
        }

        // 如果没有收到响应，返回失败结果
        TaskStatusResult result;
        result.status = NavigationStatus::FAILED;
        result.errorCode = ErrorCode::TIMEOUT;
        result.timestamp = getCurrentTimestamp();

        return result;
    }

    // 恢复原有的基于 future 的异步方法
    std::future<TaskStatusResult> queryTaskStatusAsync() {
        return std::async(std::launch::async, [this]() {
            return queryTaskStatus();
        });
    }

    // 添加基于回调的异步方法实现
    void queryTaskStatusAsync(TaskStatusResultCallback callback) {
        if (!callback) {
            return;
        }

        std::thread([this, callback = std::move(callback)]() {
            try {
                TaskStatusResult result = queryTaskStatus();
                callback(result, ErrorCode::SUCCESS);
            } catch (const std::exception& e) {
                // 创建一个空的状态结果
                TaskStatusResult emptyResult;
                emptyResult.timestamp = getCurrentTimestamp();
                callback(emptyResult, ErrorCode::FAILURE);
            }
        }).detach();
    }

    // MessageQueue接口实现
    void pushMessage(std::unique_ptr<protocol::IMessage> message) override {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        message_queue_.push(std::move(message));
        queue_cv_.notify_one();
    }

    bool popMessage(std::unique_ptr<protocol::IMessage>& message, std::chrono::milliseconds timeout) override {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // 等待队列非空或超时
        if (!queue_cv_.wait_for(lock, timeout, [this] { return !message_queue_.empty(); })) {
            return false;
        }

        // 取出消息
        message = std::move(message_queue_.front());
        message_queue_.pop();

        return true;
    }

private:
    // 等待响应消息
    bool waitForResponse(std::unique_ptr<protocol::IMessage>& response, std::chrono::milliseconds timeout) {
        return popMessage(response, timeout);
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
    std::unique_ptr<network::AsioNetworkModel> network_model_;

    // 消息队列
    std::queue<std::unique_ptr<protocol::IMessage>> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
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

// 恢复原有的基于 future 的异步方法实现
std::future<RealTimeStatus> NavigationSdk::getRealTimeStatusAsync() {
    return impl_->getRealTimeStatusAsync();
}

// 添加基于回调的异步方法实现
void NavigationSdk::getRealTimeStatusAsync(RealTimeStatusCallback callback) {
    impl_->getRealTimeStatusAsync(std::move(callback));
}

NavigationResult NavigationSdk::startNavigation(const std::vector<NavigationPoint>& points) {
    return impl_->startNavigation(points);
}

// 恢复原有的基于 future 的异步方法实现
std::future<NavigationResult> NavigationSdk::startNavigationAsync(const std::vector<NavigationPoint>& points) {
    return impl_->startNavigationAsync(points);
}

// 添加基于回调的异步方法实现
void NavigationSdk::startNavigationAsync(const std::vector<NavigationPoint>& points, NavigationResultCallback callback) {
    impl_->startNavigationAsync(points, std::move(callback));
}

bool NavigationSdk::cancelNavigation() {
    return impl_->cancelNavigation();
}

// 恢复原有的基于 future 的异步方法实现
std::future<bool> NavigationSdk::cancelNavigationAsync() {
    return impl_->cancelNavigationAsync();
}

// 添加基于回调的异步方法实现
void NavigationSdk::cancelNavigationAsync(OperationResultCallback callback) {
    impl_->cancelNavigationAsync(std::move(callback));
}

TaskStatusResult NavigationSdk::queryTaskStatus() {
    return impl_->queryTaskStatus();
}

// 恢复原有的基于 future 的异步方法实现
std::future<TaskStatusResult> NavigationSdk::queryTaskStatusAsync() {
    return impl_->queryTaskStatusAsync();
}

// 添加基于回调的异步方法实现
void NavigationSdk::queryTaskStatusAsync(TaskStatusResultCallback callback) {
    impl_->queryTaskStatusAsync(std::move(callback));
}

std::string NavigationSdk::getVersion() {
    return SDK_VERSION;
}

} // namespace dog_navigation
