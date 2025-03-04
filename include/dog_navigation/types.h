#pragma once

#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
namespace dog_navigation {

/**
 * @brief 错误码枚举
 */
enum class ErrorCode {
    SUCCESS = 0,      ///< 操作成功
    FAILURE = 1,      ///< 操作失败
    CANCELLED = 2,    ///< 操作被取消
    TIMEOUT = 3,      ///< 操作超时
    INVALID_PARAM = 4,///< 无效参数
    NOT_CONNECTED = 5,///< 未连接
    NETWORK_ERROR = 6 ///< 网络错误
};

/**
 * @brief 导航任务状态枚举
 */
enum class NavigationStatus {
    COMPLETED = 0,    ///< 任务已完成
    EXECUTING = 1,    ///< 任务执行中
    FAILED = -1       ///< 任务失败
};

/**
 * @brief 事件类型枚举
 */
enum class EventType {
    CONNECTED,           ///< 已连接
    DISCONNECTED,        ///< 已断开连接
    NAVIGATION_STARTED,  ///< 导航任务已开始
    NAVIGATION_COMPLETED,///< 导航任务已完成
    NAVIGATION_FAILED,   ///< 导航任务失败
    STATUS_UPDATED,      ///< 状态已更新
    ERROR_OCCURRED       ///< 发生错误
};

/**
 * @brief 导航点信息
 */
struct NavigationPoint {
    int mapId = 0;       ///< 地图ID
    int value = 0;       ///< 点值
    double posX = 0.0;   ///< X坐标
    double posY = 0.0;   ///< Y坐标
    double posZ = 0.0;   ///< Z坐标
    double angleYaw = 0.0;///< Yaw角度
    int pointInfo = 0;   ///< 点信息
    int gait = 0;        ///< 步态
    int speed = 0;       ///< 速度
    int manner = 0;      ///< 方式
    int obsMode = 0;     ///< 障碍物模式
    int navMode = 0;     ///< 导航模式
    int terrain = 0;     ///< 地形
    int posture = 0;     ///< 姿态

    static NavigationPoint fromJson(const nlohmann::json& json) {
        NavigationPoint point;
        point.mapId = json.value("MapID", 0);
        point.value = json.value("Value", 0);
        point.posX = json.value("PosX", 0.0);
        point.posY = json.value("PosY", 0.0);
        point.posZ = json.value("PosZ", 0.0);
        point.angleYaw = json.value("AngleYaw", 0.0);
        point.pointInfo = json.value("PointInfo", 0);
        point.gait = json.value("Gait", 0);
        point.speed = json.value("Speed", 0);
        point.manner = json.value("Manner", 0);
        point.obsMode = json.value("ObsMode", 0);
        point.navMode = json.value("NavMode", 0);
        point.terrain = json.value("Terrain", 0);
        point.posture = json.value("Posture", 0);
        return point;
    }
};


#pragma pack(push, 1)
/**
 * @brief 实时状态信息
 */
struct RealTimeStatus {
    int motionState = 0;                ///< 运动状态
    double posX = 0.0;                  ///< 位置X
    double posY = 0.0;                  ///< 位置Y
    double posZ = 0.0;                  ///< 位置Z
    double angleYaw = 0.0;              ///< 角度Yaw
    double roll = 0.0;                  ///< 角度Roll
    double pitch = 0.0;                 ///< 角度Pitch
    double yaw = 0.0;                   ///< 角度Yaw
    double speed = 0.0;                 ///< 速度
    double curOdom = 0.0;               ///< 当前里程
    double sumOdom = 0.0;               ///< 累计里程
    uint64_t curRuntime = 0;            ///< 当前运行时间
    uint64_t sumRuntime = 0;            ///< 累计运行时间
    double res = 0.0;                   ///< 响应时间
    double x0 = 0.0;                    ///< 坐标X0
    double y0 = 0.0;                    ///< 坐标Y0
    int h = 0;                          ///< 高度
    int electricity = 0;                ///< 电量
    int location = 0;                   ///< 位置  定位正常=0, 定位丢失=1
    int RTKState = 0;                   ///< RTK状态
    int onDockState = 0;                ///< 上岸状态
    int gaitState = 0;                  ///< 步态状态
    int motorState = 0;                 ///< 电机状态
    int chargeState = 0;                ///< 充电状态
    int controlMode = 0;                ///< 控制模式
    int mapUpdateState = 0;             ///< 地图更新状态
    std::string timestamp;              ///< 时间戳
};
#pragma pack(pop)
/**
 * @brief 导航任务结果
 */
struct NavigationResult {
    int value = 0;                      ///< 导航任务目标点编号，与下发导航任务请求对应
    ErrorCode errorCode = ErrorCode::SUCCESS; ///< 错误码
    int errorStatus = 0;                ///< 错误状态码
    std::string timestamp;              ///< 时间戳
};

/**
 * @brief 任务状态查询结果
 */
struct TaskStatusResult {
    int value = 0;                      ///< 导航任务目标点编号，与下发导航任务请求对应
    NavigationStatus status = NavigationStatus::COMPLETED; ///< 导航状态
    ErrorCode errorCode = ErrorCode::SUCCESS; ///< 错误码
    std::string timestamp;              ///< 时间戳
};

/**
 * @brief 事件信息
 */
struct Event {
    EventType type;                     ///< 事件类型
    std::string message;                ///< 事件消息
    std::chrono::system_clock::time_point timestamp; ///< 事件时间戳

    /**
     * @brief 将事件转换为字符串
     * @return 事件的字符串表示
     */
    std::string toString() const;
};

/**
 * @brief SDK配置选项
 */
struct SdkOptions {
    std::chrono::milliseconds connectionTimeout{5000}; ///< 连接超时时间
    std::chrono::milliseconds requestTimeout{3000};    ///< 请求超时时间
    // bool autoReconnect = true;                         ///< 是否自动重连
    // int maxReconnectAttempts = 3;                      ///< 最大重连尝试次数
    // std::chrono::milliseconds reconnectInterval{1000}; ///< 重连间隔
    // bool enableLogging = false;                        ///< 是否启用日志
    // std::string logLevel = "info";                     ///< 日志级别
};

/**
 * @brief 事件回调函数类型
 */
using EventCallback = std::function<void(const Event&)>;

/**
 * @brief 导航任务结果回调函数类型
 */
using NavigationResultCallback = std::function<void(const NavigationResult&)>;

} // namespace dog_navigation
