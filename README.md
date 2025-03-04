# X30 机器狗导航 SDK

## 简介

X30 机器狗导航 SDK 提供了一套简单易用的接口，用于控制和监控机器狗的导航任务。该 SDK 封装了底层的协议和网络通信细节，使开发者能够专注于业务逻辑的实现。

## 功能特性

- 连接和断开与机器狗控制系统的通信
- 获取机器狗的实时状态信息
- 发送导航任务指令
- 取消正在执行的导航任务
- 查询当前导航任务的执行状态
- 异步事件通知机制

## 系统要求

- C++17 或更高版本
- CMake 3.10 或更高版本
- Boost 1.66 或更高版本 (用于网络通信)
- nlohmann/json (用于JSON解析)
- rapidxml (用于XML解析)

## 安装

### 使用 CMake 构建

```bash
mkdir build && cd build
cmake ..
make
make install
```

## 快速开始

以下是一个简单的示例，展示如何使用 SDK 连接到机器狗并发送导航任务：

```cpp
#include <navigation_sdk.h>
#include <types.h>
#include <iostream>

int main() {
    // 创建 SDK 实例
    nav_sdk::NavigationSdk sdk;

    // 设置事件回调
    sdk.setEventCallback([](const nav_sdk::Event& event) {
        std::cout << "收到事件: " << event.toString() << std::endl;
    });

    // 连接到机器狗控制系统
    if (!sdk.connect("192.168.1.100", 8080)) {
        std::cerr << "连接失败!" << std::endl;
        return 1;
    }

    // 获取实时状态
    auto status = sdk.getRealTimeStatus();
    std::cout << "当前位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;

    // 创建导航点
    std::vector<nav_sdk::NavigationPoint> points;
    nav_sdk::NavigationPoint point;
    point.posX = 10.0;
    point.posY = 5.0;
    point.posZ = 0.0;
    point.angleYaw = 90.0;
    point.speed = 2;
    points.push_back(point);

    // 发送导航任务
    auto result = sdk.startNavigation(points);
    if (result.errorCode == nav_sdk::ErrorCode::SUCCESS) {
        std::cout << "导航任务已启动，任务ID: " << result.taskId << std::endl;
    } else {
        std::cerr << "导航任务启动失败，错误码: " << static_cast<int>(result.errorCode) << std::endl;
    }

    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 查询任务状态
    auto taskStatus = sdk.queryTaskStatus();
    std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;

    // 断开连接
    sdk.disconnect();

    return 0;
}
```

更多示例请参考 `examples` 目录。

## API 文档

详细的 API 文档请参考 `docs` 目录或查看头文件中的注释。

## 许可证

本项目采用 MIT 许可证。详情请参阅 LICENSE 文件。
