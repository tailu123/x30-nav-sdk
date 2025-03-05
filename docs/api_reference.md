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
    int value = 0;                      ///< 导航任务目标点编号，与下发导航任务请求对应
    ErrorCode errorCode = ErrorCode::SUCCESS; ///< 错误码
    int errorStatus = 0;                ///< 错误状态码
};
```

### CancelResult

```cpp
/**
 * @brief 取消任务结果
 */
struct CancelResult {
    uint32_t task_id;       ///< 任务 ID
    bool success;           ///< 是否成功
    std::string message;    ///< 结果消息

    /**
     * @brief 默认构造函数
     */
    CancelResult() : task_id(0), success(false) {}

    /**
     * @brief 构造函数
     * @param id 任务 ID
     * @param succ 是否成功
     * @param msg 结果消息
     */
    CancelResult(uint32_t id, bool succ, const std::string& msg = "")
        : task_id(id), success(succ), message(msg) {}
};
```

### TaskStatus

```cpp
/**
 * @brief 任务状态
 */
struct TaskStatus {
    uint32_t task_id;                       ///< 任务 ID
    TaskState state;                         ///< 任务状态
    double progress;                         ///< 任务进度（百分比，0-100）
    std::string message;                     ///< 状态消息
    std::vector<NavigationPoint> completed;  ///< 已完成的导航点
    std::vector<NavigationPoint> remaining;  ///< 剩余的导航点

    /**
     * @brief 默认构造函数
     */
    TaskStatus() : task_id(0), state(TaskState::UNKNOWN), progress(0) {}

    /**
     * @brief 构造函数
     * @param id 任务 ID
     * @param st 任务状态
     * @param prog 任务进度
     * @param msg 状态消息
     */
    TaskStatus(uint32_t id, TaskState st, double prog, const std::string& msg = "")
        : task_id(id), state(st), progress(prog), message(msg) {}
};
```

### Event

```cpp
/**
 * @brief 事件
 */
struct Event {
    EventType type;         ///< 事件类型
    std::string message;    ///< 事件消息
    uint32_t task_id;       ///< 相关任务 ID（如果适用）

    /**
     * @brief 默认构造函数
     */
    Event() : type(EventType::UNKNOWN), task_id(0) {}

    /**
     * @brief 构造函数
     * @param t 事件类型
     * @param msg 事件消息
     * @param id 相关任务 ID（如果适用）
     */
    Event(EventType t, const std::string& msg = "", uint32_t id = 0)
        : type(t), message(msg), task_id(id) {}
};
```

## 枚举类型

### ErrorCode

```cpp
/**
 * @brief 错误码
 */
enum class ErrorCode {
    SUCCESS = 0,            ///< 成功
    NOT_CONNECTED,          ///< 未连接
    TIMEOUT,                ///< 超时
    INVALID_PARAMETER,      ///< 无效参数
    SEND_FAILED,            ///< 发送失败
    RECEIVE_FAILED,         ///< 接收失败
    PARSE_FAILED,           ///< 解析失败
    TASK_NOT_FOUND,         ///< 任务未找到
    SERVER_ERROR,           ///< 服务器错误
    UNKNOWN_ERROR           ///< 未知错误
};
```

### DogStatus

```cpp
/**
 * @brief 机器狗状态
 */
enum class DogStatus {
    UNKNOWN,                ///< 未知
    IDLE,                   ///< 空闲
    NAVIGATING,             ///< 导航中
    ERROR,                  ///< 错误
    LOW_BATTERY,            ///< 低电量
    CHARGING                ///< 充电中
};
```

### TaskState

```cpp
/**
 * @brief 任务状态
 */
enum class TaskState {
    UNKNOWN,                ///< 未知
    PENDING,                ///< 等待中
    RUNNING,                ///< 运行中
    PAUSED,                 ///< 暂停
    COMPLETED,              ///< 已完成
    CANCELLED,              ///< 已取消
    FAILED                  ///< 失败
};
```

### EventType

```cpp
/**
 * @brief 事件类型
 */
enum class EventType {
    UNKNOWN,                ///< 未知
    CONNECTION_ESTABLISHED, ///< 连接已建立
    DISCONNECTED,           ///< 已断开连接
    TASK_STARTED,           ///< 任务已开始
    TASK_COMPLETED,         ///< 任务已完成
    TASK_CANCELLED,         ///< 任务已取消
    TASK_FAILED,            ///< 任务失败
    LOW_BATTERY,            ///< 低电量
    ERROR                   ///< 错误
};
```

### MessageType

```cpp
/**
 * @brief 消息类型
 */
enum class MessageType {
    UNKNOWN,                        ///< 未知
    GET_REAL_TIME_STATUS,           ///< 获取实时状态
    REAL_TIME_STATUS_RESPONSE,      ///< 实时状态响应
    START_NAVIGATION,               ///< 开始导航
    START_NAVIGATION_RESPONSE,      ///< 开始导航响应
    CANCEL_NAVIGATION,              ///< 取消导航
    CANCEL_NAVIGATION_RESPONSE,     ///< 取消导航响应
    QUERY_TASK_STATUS,              ///< 查询任务状态
    TASK_STATUS_RESPONSE,           ///< 任务状态响应
    EVENT_NOTIFICATION              ///< 事件通知
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

### RealTimeStatusCallback

```cpp
/**
 * @brief 实时状态回调函数类型
 * @param status 实时状态
 * @param error 错误码
 */
using RealTimeStatusCallback = std::function<void(const RealTimeStatus& status, ErrorCode error)>;
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

### CancelResultCallback

```cpp
/**
 * @brief 取消结果回调函数类型
 * @param result 取消结果
 * @param error 错误码
 */
using CancelResultCallback = std::function<void(const CancelResult& result, ErrorCode error)>;
```

### TaskStatusCallback

```cpp
/**
 * @brief 任务状态回调函数类型
 * @param status 任务状态
 * @param error 错误码
 */
using TaskStatusCallback = std::function<void(const TaskStatus& status, ErrorCode error)>;
```

## 辅助函数

### getErrorMessage

```cpp
/**
 * @brief 获取错误码对应的错误消息
 * @param error 错误码
 * @return 错误消息
 */
std::string getErrorMessage(ErrorCode error);
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
    if (sdk.connect("192.168.1.100", 8080)) {
        std::cout << "连接请求已发送" << std::endl;
    } else {
        std::cout << "连接请求发送失败" << std::endl;
        return 1;
    }

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (sdk.isConnected()) {
        // 获取实时状态
        auto [status, error] = sdk.getRealTimeStatus();
        if (error == nav_sdk::ErrorCode::SUCCESS) {
            std::cout << "纬度: " << status.latitude << ", 经度: " << status.longitude << std::endl;
            std::cout << "电池电量: " << status.battery_level << "%" << std::endl;
        } else {
            std::cout << "获取实时状态失败: " << nav_sdk::getErrorMessage(error) << std::endl;
        }

        // 创建导航点
        std::vector<nav_sdk::NavigationPoint> points = {
            {39.9042, 116.4074, 0, 1.0, 90.0},  // 北京
            {31.2304, 121.4737, 0, 1.0, 180.0}  // 上海
        };

        // 开始导航任务
        auto [result, nav_error] = sdk.startNavigation(points);
        if (nav_error == nav_sdk::ErrorCode::SUCCESS) {
            std::cout << "导航任务已开始，任务 ID: " << result.task_id << std::endl;

            // 查询任务状态
            auto [task_status, task_error] = sdk.queryTaskStatus(result.task_id);
            if (task_error == nav_sdk::ErrorCode::SUCCESS) {
                std::cout << "任务状态: " << static_cast<int>(task_status.state) << std::endl;
                std::cout << "任务进度: " << task_status.progress << "%" << std::endl;
            }

            // 取消任务
            auto [cancel_result, cancel_error] = sdk.cancelNavigation(result.task_id);
            if (cancel_error == nav_sdk::ErrorCode::SUCCESS) {
                std::cout << "任务已取消" << std::endl;
            }
        } else {
            std::cout << "开始导航任务失败: " << nav_sdk::getErrorMessage(nav_error) << std::endl;
        }
    }

    // 断开连接
    sdk.disconnect();

    return 0;
}
```

### 异步方法示例

```cpp
#include <navigation_sdk.h>
#include <iostream>
#include <condition_variable>

int main() {
    // 创建 SDK 实例
    nav_sdk::NavigationSdk sdk;

    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;

    // 设置事件回调
    sdk.setEventCallback([&](const nav_sdk::Event& event) {
        std::cout << "事件: " << static_cast<int>(event.type) << ", 消息: " << event.message << std::endl;

        if (event.type == nav_sdk::EventType::CONNECTION_ESTABLISHED) {
            // 连接已建立，获取实时状态
            sdk.getRealTimeStatusAsync([&](const nav_sdk::RealTimeStatus& status, nav_sdk::ErrorCode error) {
                if (error == nav_sdk::ErrorCode::SUCCESS) {
                    std::cout << "纬度: " << status.latitude << ", 经度: " << status.longitude << std::endl;
                    std::cout << "电池电量: " << status.battery_level << "%" << std::endl;

                    // 创建导航点
                    std::vector<nav_sdk::NavigationPoint> points = {
                        {39.9042, 116.4074, 0, 1.0, 90.0},  // 北京
                        {31.2304, 121.4737, 0, 1.0, 180.0}  // 上海
                    };

                    // 开始导航任务
                    sdk.startNavigationAsync(points, [&](const nav_sdk::NavigationResult& result, nav_sdk::ErrorCode nav_error) {
                        if (nav_error == nav_sdk::ErrorCode::SUCCESS) {
                            std::cout << "导航任务已开始，任务 ID: " << result.task_id << std::endl;

                            // 查询任务状态
                            sdk.queryTaskStatusAsync(result.task_id, [&](const nav_sdk::TaskStatus& task_status, nav_sdk::ErrorCode task_error) {
                                if (task_error == nav_sdk::ErrorCode::SUCCESS) {
                                    std::cout << "任务状态: " << static_cast<int>(task_status.state) << std::endl;
                                    std::cout << "任务进度: " << task_status.progress << "%" << std::endl;

                                    // 取消任务
                                    sdk.cancelNavigationAsync(result.task_id, [&](const nav_sdk::CancelResult& cancel_result, nav_sdk::ErrorCode cancel_error) {
                                        if (cancel_error == nav_sdk::ErrorCode::SUCCESS) {
                                            std::cout << "任务已取消" << std::endl;
                                        }

                                        // 断开连接
                                        sdk.disconnect();

                                        // 通知主线程完成
                                        {
                                            std::lock_guard<std::mutex> lock(mutex);
                                            done = true;
                                        }
                                        cv.notify_one();
                                    });
                                }
                            });
                        }
                    });
                }
            });
        }
    });

    // 连接到机器狗控制系统
    if (sdk.connect("192.168.1.100", 8080)) {
        std::cout << "连接请求已发送" << std::endl;
    } else {
        std::cout << "连接请求发送失败" << std::endl;
        return 1;
    }

    // 等待完成
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&]() { return done; });
    }

    return 0;
}
```

## 错误处理

SDK 使用 `ErrorCode` 枚举表示操作结果。每个同步方法都返回一个包含结果和错误码的对，每个异步方法都在回调函数中提供结果和错误码。

```cpp
// 同步方法错误处理
auto [status, error] = sdk.getRealTimeStatus();
if (error != nav_sdk::ErrorCode::SUCCESS) {
    std::cerr << "获取实时状态失败: " << nav_sdk::getErrorMessage(error) << std::endl;
    // 处理错误
}

// 异步方法错误处理
sdk.getRealTimeStatusAsync([](const nav_sdk::RealTimeStatus& status, nav_sdk::ErrorCode error) {
    if (error != nav_sdk::ErrorCode::SUCCESS) {
        std::cerr << "获取实时状态失败: " << nav_sdk::getErrorMessage(error) << std::endl;
        // 处理错误
    }
});
```

## 线程安全性

SDK 的所有公共 API 都是线程安全的，可以从多个线程同时调用。回调函数在 IO 线程中执行，不应执行长时间操作。

## 版本历史

| 版本 | 发布日期 | 主要变更 |
|------|----------|---------|
| 0.1.0 | 2025-03-05 | 初始版本 |
| 1.0.0 | 2025-03-05 | 稳定版本发布 |
