#include <dog_navigation/navigation_sdk.h>
#include <dog_navigation/types.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>

/**
 * @brief 打印实时状态信息
 * @param status 实时状态信息
 */
void printStatus(const dog_navigation::RealTimeStatus& status) {
    std::cout << "===== 实时状态信息 =====" << std::endl;
    std::cout << "位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;
    std::cout << "角度: " << status.angleYaw << "°" << std::endl;
    std::cout << "速度: " << status.speed << std::endl;
    std::cout << "电量: " << status.electricity << "%" << std::endl;
    std::cout << "运动状态: " << status.motionState << std::endl;
    std::cout << "========================" << std::endl;
}

/**
 * @brief 打印任务状态信息
 * @param status 任务状态查询结果
 */
void printTaskStatus(const dog_navigation::TaskStatusResult& status) {
    std::cout << "===== 任务状态信息 =====" << std::endl;
    std::cout << "目标点编号: " << status.value << std::endl;
    std::cout << "状态: ";

    switch (status.status) {
        case dog_navigation::NavigationStatus::COMPLETED:
            std::cout << "已完成" << std::endl;
            break;
        case dog_navigation::NavigationStatus::EXECUTING:
            std::cout << "执行中" << std::endl;
            break;
        case dog_navigation::NavigationStatus::FAILED:
            std::cout << "失败" << std::endl;
            break;
    }

    std::cout << "错误码: " << static_cast<int>(status.errorCode) << std::endl;
    std::cout << "时间戳: " << status.timestamp << std::endl;
    std::cout << "========================" << std::endl;
}

/**
 * @brief 打印导航结果
 * @param result 导航任务结果
 */
void printNavigationResult(const dog_navigation::NavigationResult& result) {
    std::cout << "===== 导航任务结果 =====" << std::endl;
    std::cout << "目标点编号: " << result.value << std::endl;
    std::cout << "错误码: " << static_cast<int>(result.errorCode) << std::endl;
    std::cout << "错误状态: " << result.errorStatus << std::endl;
    std::cout << "时间戳: " << result.timestamp << std::endl;
    std::cout << "========================" << std::endl;
}

/**
 * @brief 事件回调函数
 * @param event 事件信息
 */
void onEvent(const dog_navigation::Event& event) {
    std::cout << "收到事件: " << event.toString() << std::endl;
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 3) {
        std::cerr << "用法: " << argv[0] << " <主机地址> <端口>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    std::cout << "X30 机器狗导航 SDK 示例程序" << std::endl;
    std::cout << "SDK 版本: " << dog_navigation::NavigationSdk::getVersion() << std::endl;
    std::cout << "连接到: " << host << ":" << port << std::endl;

    try {
        // 创建 SDK 实例
        dog_navigation::SdkOptions options;
        options.connectionTimeout = std::chrono::milliseconds(3000);
        options.requestTimeout = std::chrono::milliseconds(3000);
        options.navigationAsyncTimeout = std::chrono::minutes(3);
        options.enableLogging = true;

        dog_navigation::NavigationSdk sdk(options);

        // 设置事件回调
        sdk.setEventCallback(onEvent);

        // 连接到机器狗控制系统
        if (!sdk.connect(host, port)) {
            std::cerr << "连接失败!" << std::endl;
            return 1;
        }

        std::cout << "连接成功!" << std::endl;

        // 获取初始实时状态
        // auto status = sdk.getRealTimeStatus();
        // printStatus(status);

        // 创建导航点
        std::vector<dog_navigation::NavigationPoint> points;
        dog_navigation::NavigationPoint point;
        point.posX = 10.0;
        point.posY = 5.0;
        point.posZ = 0.0;
        point.angleYaw = 90.0;
        point.speed = 2;
        points.push_back(point);

        // 标记是否收到导航响应
        std::atomic<bool> navigationResponseReceived{false};

        // 异步发送导航任务（使用回调形式）
        std::cout << "开始导航任务..." << std::endl;
        sdk.startNavigationAsync(points, [&](const dog_navigation::NavigationResult& result) {
            // 打印导航任务结果
            printNavigationResult(result);

            // 标记已收到导航响应
            navigationResponseReceived = true;
        });

        // 轮询查询任务状态和实时状态，直到收到导航响应
        int pollCount = 0;
        const int MAX_POLL_COUNT = 30; // 最多轮询30次
        const auto POLL_INTERVAL = std::chrono::milliseconds(1000); // 轮询间隔1秒

        while (!navigationResponseReceived && pollCount < MAX_POLL_COUNT) {
            std::this_thread::sleep_for(POLL_INTERVAL);
            pollCount++;

            std::cout << "\n轮询 #" << pollCount << ":" << std::endl;

            // 查询任务状态
            auto taskStatus = sdk.queryTaskStatus();
            printTaskStatus(taskStatus);

            // 获取实时状态
            auto status = sdk.getRealTimeStatus();
            printStatus(status);
        }

        // 方式1不关心取消导航结果
        // if (!navigationResponseReceived) {
        //     std::cout << "达到最大轮询次数，尝试取消任务..." << std::endl;
        //     sdk.cancelNavigationAsync();

        //     // 等待一小段时间，确保取消请求被处理
        //     std::this_thread::sleep_for(std::chrono::seconds(2));
        // }

        // 方式2关心取消导航结果
        // if (!navigationResponseReceived) {
        //     std::cout << "达到最大轮询次数，尝试取消任务..." << std::endl;

        //     sdk.cancelNavigationAsync([](bool result, dog_navigation::ErrorCode errorCode) {
        //         std::cout << "取消导航任务结果: " << (result ? "成功" : "失败") 
        //             << ", 错误码: " << static_cast<int>(errorCode) << std::endl;
        //     });

        //     // 等待一小段时间，确保取消请求被处理
        //     std::this_thread::sleep_for(std::chrono::seconds(2));
        // }

        if (!navigationResponseReceived) {
            std::cout << "达到最大轮询次数，尝试取消任务..." << std::endl;
            if (sdk.cancelNavigation()) {
                std::cout << "导航任务已取消" << std::endl;
            } else {
                std::cout << "导航任务取消失败" << std::endl;
            }
        }

        // 等待2秒
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 断开连接
        sdk.disconnect();
        std::cout << "已断开连接" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
