# 快速入门指南

本指南将帮助您快速上手使用 X30 机器狗导航 SDK，即使您不熟悉 C++ 编程语言。

## 系统要求

在开始使用 SDK 之前，请确保您的系统满足以下要求：

- 操作系统：Linux、macOS 或 Windows
- C++17 或更高版本的编译器
- CMake 3.10 或更高版本
- Boost 1.66 或更高版本
- nlohmann/json 库
- rapidxml 库

## 安装 SDK

### 使用预编译的二进制包

1. 下载适合您系统的预编译二进制包
2. 解压到您的项目目录
3. 在您的项目中包含 SDK 的头文件和库文件

### 从源代码构建

1. 克隆 SDK 代码库：

```bash
git clone https://github.com/tailu123/x30-nav-sdk.git
cd x30_nav_sdk
```

2. 创建构建目录并编译：

```bash
mkdir build && cd build
cmake ..
make
```

3. 安装 SDK（可选）：

```bash
sudo make install
```

## 基本用法

以下是使用 SDK 的基本步骤：

### 1. 包含头文件

```cpp
#include <navigation_sdk.h>
```

### 2. 创建 SDK 实例

```cpp
// 创建 SDK 实例
nav_sdk::NavigationSdk sdk;
```

### 3. 连接到机器狗控制系统

```cpp
// 连接到机器狗控制系统
if (!sdk.connect("192.168.1.106", 30000)) {
    std::cerr << "连接失败!" << std::endl;
    return 1;
}
```

### 4. 获取实时状态

```cpp
// 获取实时状态
auto status = sdk.request1002_RunTimeStatus();
std::cout << "当前位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;
```

### 5. 发送导航任务

```cpp
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
sdk.request1003_StartNavTask(points, [](void(const NavigationResult& navigationResult)) {
    if (navigationResult.errorCode == ErrorCode_Navigation::SUCCESS) {
        std::cout << "导航任务成功完成!" << std::endl;
    } else {
        std::cout << "导航任务失败, errorStatus: " << static_cast<int>(navigationResult.errorStatus) << std::endl;
    }
});
```

### 6. 查询任务状态

```cpp
// 查询任务状态
auto taskStatus = sdk.request1007_NavTaskStatus();
std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;
```

### 7. 取消导航任务

```cpp
// 取消导航任务
if (sdk.request1004_CancelNavTask()) {
    std::cout << "导航任务已取消" << std::endl;
} else {
    std::cerr << "导航任务取消失败" << std::endl;
}
```

### 8. 断开连接

```cpp
// 断开连接
sdk.disconnect();
```

## 完整示例

以下是一个完整的示例程序，展示了如何使用 SDK 的基本功能：

```cpp
#include <navigation_sdk.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 创建 SDK 实例
    nav_sdk::NavigationSdk sdk;

    // 连接到机器狗控制系统
    if (!sdk.connect("192.168.1.106", 30000)) {
        std::cerr << "连接失败!" << std::endl;
        return 1;
    }

    // 获取实时状态
    auto status = sdk.request1002_RunTimeStatus();
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
    sdk.request1003_StartNavTask(points, [](void(const NavigationResult& navigationResult)) {
        if (navigationResult.errorCode == ErrorCode_Navigation::SUCCESS) {
            std::cout << "导航任务已启动，任务ID: " << navigationResult.value << std::endl;
        } else {
            std::cerr << "导航任务启动失败，错误码: " << static_cast<int>(navigationResult.errorCode) << std::endl;
        }
    });

    // 等待5秒
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 查询任务状态
    auto taskStatus = sdk.request1007_NavTaskStatus();
    std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;

    // 取消导航任务
    if (sdk.request1004_CancelNavTask()) {
        std::cout << "导航任务已取消" << std::endl;
    } else {
        std::cerr << "导航任务取消失败" << std::endl;
    }

    // 断开连接
    sdk.disconnect();

    return 0;
}
```

## 下一步

- 查看 [API 参考](api_reference.md) 了解更多 SDK 功能
- 阅读 [最佳实践](best_practices.md) 获取使用建议
- 参考 [错误处理](error_handling.md) 了解如何处理常见错误
