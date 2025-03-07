# API 参考

本文档提供 X30 机器狗导航 SDK 的详细 API 参考，包括所有公共类、方法、枚举和数据结构的说明。

## 目录

- [NavigationSdk 类](#navigationsdk-类)
- [数据类型](#数据类型)
- [枚举类型](#枚举类型)
- [回调函数](#回调函数)

## NavigationSdk 类

`NavigationSdk` 是 SDK 的主要接口类，提供与机器狗控制系统通信的所有功能。

### 构造函数和析构函数

```cpp
/**
 * @brief 创建 NavigationSdk 实例
 */
NavigationSdk();

/**
 * @brief 销毁 NavigationSdk 实例，释放所有资源
 */
~NavigationSdk();
```

### 连接管理

```cpp
/**
 * @brief 连接到机器狗控制系统
 * @param host 主机地址（IP 或域名）
 * @param port 端口号
 * @return 如果连接请求已成功发送，则返回 true；否则返回 false
 * @note 连接结果将通过 EventCallback 通知
 */
bool connect(const std::string& host, uint16_t port);

/**
 * @brief 断开与机器狗控制系统的连接
 */
void disconnect();

/**
 * @brief 检查是否已连接到机器狗控制系统
 * @return 如果已连接，则返回 true；否则返回 false
 */
bool isConnected() const;
```

### 事件回调

```cpp
/**
 * @brief 设置事件回调函数
 * @param callback 事件回调函数
 * @note 事件回调函数在 IO 线程中调用，不应执行长时间操作
 */
void setEventCallback(EventCallback callback);
```

### 实时状态

```cpp
/**
 * @brief 获取机器狗实时状态（同步方法）
 * @return 包含实时状态如位置、速度、角度、电量等
 */
RealTimeStatus getRealTimeStatus();
```

### 导航任务

```cpp
/**
 * @brief 开始导航任务（异步方法）
 * @param navigation_points 导航点列表
 * @param callback 结果回调函数
 * @note 导航任务完成后，会通过回调函数返回结果; 回调函数在IO线程中调用，不应执行长时间操作
 */
void startNavigationAsync(
    const std::vector<NavigationPoint>& navigation_points,
    NavigationResultCallback callback);

/**
 * @brief 取消导航任务（同步方法）
 * @return 如果取消成功，则返回 true；否则返回 false
 */
bool cancelNavigation();

/**
 * @brief 查询任务状态（同步方法）
 * @return 包含任务状态和错误码
 */
TaskStatusResult queryTaskStatus();
```

### 版本信息

```cpp
/**
 * @brief 获取 SDK 版本信息
 * @return SDK 版本字符串
 */
static std::string getVersion();
```

## 数据类型

### NavigationPoint

```cpp
/**
 * @brief 导航点
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
};
```

### RealTimeStatus

```cpp
/**
 * @brief 机器狗实时状态
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
};
```

### NavigationResult

```cpp
/**
 * @brief 导航任务结果
 */
struct NavigationResult {
    int value = 0;                                                  ///< 导航任务目标点编号，与下发导航任务请求对应
    ErrorCode_Navigation errorCode = ErrorCode_Navigation::SUCCESS; ///< 错误码 0:成功; 1:失败; 2:取消
    ErrorStatus errorStatus = ErrorStatus::DEFAULT;                 ///< 错误状态码; 导航任务失败的具体原因
};
```

### TaskStatusResult

```cpp
/**
 * @brief 任务状态查询结果
 */
struct TaskStatusResult {
    int value = 0;                                                      ///< 导航任务目标点编号，与下发导航任务请求对应
    NavigationStatus status = NavigationStatus::COMPLETED;              ///< 导航状态:  0:已完成; 1:执行中; 2:失败
    ErrorCode_QueryStatus errorCode = ErrorCode_QueryStatus::COMPLETED; ///< 错误码:   0:成功; 1:执行中; 2:失败
};
```

### Event

```cpp
/**
 * @brief 事件信息
 */
struct Event {
    EventType type;                                     ///< 事件类型
    std::string message;                                ///< 事件消息
    std::chrono::system_clock::time_point timestamp;    ///< 事件时间戳
};
```

## 枚举类型

### ErrorCode_Navigation

```cpp
/**
 * @brief 导航任务响应ErrorCode枚举
 */
enum class ErrorCode_Navigation {
    SUCCESS = 0,      ///< 操作成功
    FAILURE = 1,      ///< 操作失败
    CANCELLED = 2,    ///< 操作被取消

    INVALID_PARAM = 3,///< 无效参数
    NOT_CONNECTED = 4 ///< 未连接
};
```

### ErrorStatus_Navigation

```cpp
/**
 * @brief 导航任务响应ErrorStatus枚举
 */
enum class ErrorStatus_Navigation {
    DEFAULT = 0,                                                                 ///< 默认值
    SINGLE_POINT_INSPECTION_TASK_CANCELLED = 8962,                               ///< 单点巡检任务被取消
    SINGLE_POINT_INSPECTION_TASK_COMPLETED = 8960,                               ///< 单点巡检任务执行完成
    MOTION_STATE_EXCEPTION_FAILED = 41729,                                       ///< 运动状态异常，任务失败 (软急停、摔倒)
    LOW_POWER_FAILED = 41730,                                                    ///< 电量过低，任务失败
    MOTOR_OVER_TEMPERATURE_EXCEPTION_FAILED = 41731,                             ///< 电机过温异常，任务失败
    USING_CHARGER_CHARGING_FAILED = 41732,                                       ///< 正在使用充电器充电，任务失败
    NAVIGATION_PROCESS_NOT_STARTED_FAILED = 41745,                               ///< 导航进程未启动，无法下发任务
    NAVIGATION_MODULE_COMMUNICATION_EXCEPTION_FAILED = 41746,                    ///< 导航模块通讯异常，无法下发任务
    POSITION_STATE_CONTINUOUSLY_EXCEPTION_FAILED = 41747,                        ///< 定位状态持续异常 (超过 30s)
    TERRAIN_MODULE_STATE_EXCEPTION_FAILED = 41748,                               ///< 地形模块状态异常
    STAND_UP_FAILED = 41761,                                                     ///< 发送起立失败
    EXECUTE_STAND_UP_FAILED = 41762,                                             ///< 执行起立失败
    SWITCH_FORCE_CONTROL_FAILED = 41763,                                         ///< 切换力控失败
    SWITCH_WALKING_MODE_FAILED = 41764,                                          ///< 切换行走模式失败
    PUP_FAILED = 41765,                                                          ///< 趴下失败
    SOFT_EMERGENCY_STOP_FAILED = 41766,                                          ///< 被软急停
    SWITCH_GAIT_FAILED = 41767,                                                  ///< 切换步态失败
    SWITCH_NAVIGATION_MODE_FAILED = 41768,                                       ///< 切换导航模式失败
    SWITCH_MANUAL_MODE_FAILED = 41769,                                           ///< 切换手动模式失败
    SWITCH_NORMAL_OR_CRAWL_HEIGHT_STATE_FAILED = 41770,                          ///< 切换正常 / 匍匐身高状态失败
    SWITCH_STOP_AVOIDANCE_MODULE_SPEED_INPUT_SOURCE_FAILED = 41777,              ///< 切换停避障模块的速度输入源失败
    SET_TERRAIN_MAP_PARAMETER_FAILED = 41778,                                    ///< 设置地形图参数失败
    CURRENTLY_EXECUTING_TASK_FAILED = 41793,                                     ///< 当前正在执行任务，下发新任务失败
    SCHEDULE_EXIT_SELF_CHARGING_FAILED = 41794,                                  ///< 调度退出自主充电失败
    EXIT_SELF_CHARGING_EXECUTION_FAILED = 41795,                                 ///< 退出自主充电执行失败
    SCHEDULE_ENTER_SELF_CHARGING_FAILED = 41796,                                 ///< 调度进入自主充电失败
    ENTER_SELF_CHARGING_EXECUTION_FAILED = 41797,                                ///< 进入自主充电执行失败
    EXIT_PILE_RELOCATION_FAILED = 41798,                                         ///< 退桩后重定位失败
    OPEN_ACCUMULATION_FRAME_FAILED = 41799,                                      ///< 开启累积帧失败
    CLOSE_ACCUMULATION_FRAME_FAILED = 41800,                                     ///< 关闭累积帧失败
    SWITCH_MAP_FAILED = 41801,                                                   ///< 切换地图失败
    EXIST_UPPER_MACHINE_CONNECTION_DISCONNECTED_AUTO_STOP_TASK_FAILED = 41802,   ///< 存在上位机连接断开，自动停止任务
    STOP_AVOIDANCE_MODULE_STATE_EXCEPTION_FAILED = 41803,                        ///< 持续停障异常，导航失败
    NAVIGATION_GLOBAL_PLANNING_FAILED = 41804,                                   ///< 导航全局规划失败
    NAVIGATION_CONTINUOUS_NAVIGATION_SPEED_NOT_REFRESHED_FAILED = 41805,         ///< 持续导航速度未刷新，导航失败
    SELF_CHARGING_PROCESS_FAILED = 41806,                                        ///< 自主充电流程中，下发任务失败
    RELOCATION_FAILED = 41881,                                                   ///< 重定位失败
    PROCESS_MANUAL_RESTART_STOP_TASK_FAILED = 41983                              ///< 进程手动重启中，停止任务
};
```

### ErrorCode_QueryStatus

```cpp
/**
 * @brief 任务状态查询ErrorCode枚举
 */
enum class ErrorCode_QueryStatus {
    COMPLETED = 0,          ///< 任务已完成
    EXECUTING = 1,          ///< 任务执行中
    FAILED = -1,            ///< 无法执行

    INVALID_RESPONSE = 2,   ///< 无效响应
    TIMEOUT = 3,            ///< 超时
    NOT_CONNECTED = 4,      ///< 未连接
    UNKNOWN_ERROR = 5       ///< 未知错误
};
```

### Status_QueryStatus

```cpp
/**
 * @brief 任务状态查询status枚举
 */
enum class Status_QueryStatus {
    COMPLETED = 0,    ///< 任务已完成
    EXECUTING = 1,    ///< 任务执行中
    FAILED = -1       ///< 无法执行
};
```

### ErrorCode_RealTimeStatus

```cpp
/**
 * @brief 实时状态查询ErrorCode枚举
 */
enum class ErrorCode_RealTimeStatus {
    SUCCESS = 0,            ///< 操作成功

    INVALID_RESPONSE = 1,   ///< 无效响应
    TIMEOUT = 2,            ///< 超时
    NOT_CONNECTED = 3,      ///< 未连接
    UNKNOWN_ERROR = 4       ///< 未知错误
};
```

### EventType

```cpp
/**
 * @brief 事件类型枚举
 */
enum class EventType {
    CONNECTED,           ///< 已连接
    DISCONNECTED,        ///< 已断开连接
};
```

### MessageType

```cpp
/**
 * @brief 消息类型枚举
 */
enum class MessageType {
    UNKNOWN = 0,
    GET_REAL_TIME_STATUS_REQ,       // type=1002, command=1
    GET_REAL_TIME_STATUS_RESP,      // type=1002, command=1
    NAVIGATION_TASK_REQ,            // type=1003, command=1
    NAVIGATION_TASK_RESP,           // type=1003, command=1
    CANCEL_TASK_REQ,                // type=1004, command=1
    CANCEL_TASK_RESP,               // type=1004, command=1
    QUERY_STATUS_REQ,               // type=1007, command=1
    QUERY_STATUS_RESP               // type=1007, command=1
};
```

## 回调函数

### EventCallback

```cpp
/**
 * @brief 事件回调函数类型
 * @param event 事件
 */
using EventCallback = std::function<void(const Event& event)>;
```

### NavigationResultCallback

```cpp
/**
 * @brief 导航结果回调函数类型
 * @param result 导航结果
 * @param error 错误码
 */
using NavigationResultCallback = std::function<void(const NavigationResult& result, ErrorCode error)>;
```

## 使用示例

### 同步方法示例

```cpp
#include <navigation_sdk.h>
#include <iostream>

int main() {
    // 创建 SDK 实例
    nav_sdk::NavigationSdk sdk;

    // 设置事件回调
    sdk.setEventCallback([](const nav_sdk::Event& event) {
        std::cout << "事件: " << static_cast<int>(event.type) << ", 消息: " << event.message << std::endl;
    });

    // 连接到机器狗控制系统
    if (sdk.connect("192.168.1.106", 30000)) {
        std::cout << "连接请求已发送" << std::endl;
    } else {
        std::cout << "连接请求发送失败" << std::endl;
        return 1;
    }

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (sdk.isConnected()) {
        // 获取实时状态
        auto realTimeStatus = sdk.getRealTimeStatus();
        if (realTimeStatus.errorCode == ErrorCode_RealTimeStatus::SUCCESS) {
            std::cout << "纬度: " << realTimeStatus.latitude << ", 经度: " << realTimeStatus.longitude << std::endl;
            std::cout << "电池电量: " << realTimeStatus.battery_level << "%" << std::endl;
        } else {
            std::cout << "获取实时状态失败: " << static_cast<int>(realTimeStatus.errorCode) << std::endl;
        }

        // 创建导航点
        std::vector<nav_sdk::NavigationPoint> points = {
            {39.9042, 116.4074, 0, 1.0, 90.0},  // 北京
            {31.2304, 121.4737, 0, 1.0, 180.0}  // 上海
        };

        // 开始导航任务
        sdk.startNavigationAsync(points, [](void(const NavigationResult& navigationResult)) {
            if (navigationResult.errorCode == ErrorCode_Navigation::SUCCESS) {
                std::cout << "导航任务成功完成!" << std::endl;
            } else {
                std::cout << "导航任务失败, errorStatus: " << static_cast<int>(navigationResult.errorStatus) << std::endl;
            }
        });


        // 查询任务状态
        auto taskStatus = sdk.queryTaskStatus();
        std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;

        // 取消任务
        auto cancelResult = sdk.cancelNavigation();
        if (cancelResult) {
            std::cout << "任务已取消" << std::endl;
        } else {
            std::cout << "取消任务失败"<< std::endl;
        }
    }

    // 断开连接
    sdk.disconnect();

    return 0;
}
```

## 错误处理

SDK 使用 `ErrorCode` 枚举表示操作结果。每个同步方法都返回一个包含结果和错误码的结构体，每个异步方法都在回调函数中提供结果和错误码。

## 线程安全性

SDK 的所有公共 API 都是线程安全的，可以从多个线程同时调用。回调函数在 IO 线程中执行，不应执行长时间操作。

## 版本历史

| 版本 | 发布日期 | 主要变更 |
|------|----------|---------|
| 0.1.0 | 2025-03-05 | 初始版本 |
