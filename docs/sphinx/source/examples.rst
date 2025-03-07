使用示例
========

本文档提供了一些使用 RobotServer SDK 的示例代码，帮助您快速上手。

基础示例
-------

以下是一个基础示例，展示了如何使用 SDK 连接到机器狗并发送导航任务：

.. code-block:: cpp

   #include <iostream>
   #include <robotserver_sdk.h>
   #include <vector>
   #include <thread>
   #include <chrono>

   using namespace robotserver_sdk;

   // 默认导航点
   std::vector<NavigationPoint> createDefaultNavigationPoints() {
       std::vector<NavigationPoint> points;

       NavigationPoint point1;
       point1.mapId = 1;
       point1.value = 1;
       point1.posX = 10.0;
       point1.posY = 20.0;
       point1.posZ = 0.0;
       point1.angleYaw = 0.0;
       point1.gait = 1;
       point1.speed = 2;
       points.push_back(point1);

       NavigationPoint point2;
       point2.mapId = 1;
       point2.value = 2;
       point2.posX = 15.0;
       point2.posY = 25.0;
       point2.posZ = 0.0;
       point2.angleYaw = 90.0;
       point2.gait = 1;
       point2.speed = 2;
       points.push_back(point2);

       return points;
   }

   int main(int argc, char* argv[]) {
       if (argc < 3) {
           std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
           return 1;
       }

       std::string host = argv[1];
       int port = std::stoi(argv[2]);

       // 创建 SDK 实例
       RobotServerSdk sdk;

       // 连接到机器狗控制系统
       std::cout << "正在连接到 " << host << ":" << port << "..." << std::endl;
       if (!sdk.connect(host, port)) {
           std::cerr << "连接失败!" << std::endl;
           return 1;
       }
       std::cout << "连接成功!" << std::endl;

       // 获取实时状态
       std::cout << "获取实时状态..." << std::endl;
       auto status = sdk.request1002_RunTimeStatus();
       std::cout << "当前位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;
       std::cout << "电量: " << status.electricity << "%" << std::endl;

       // 创建导航点
       auto points = createDefaultNavigationPoints();

       // 发送导航任务
       std::cout << "发送导航任务..." << std::endl;
       bool taskCompleted = false;
       sdk.request1003_StartNavTask(points, [&taskCompleted](const NavigationResult& navigationResult) {
           if (navigationResult.errorCode == ErrorCode_Navigation::SUCCESS) {
               std::cout << "导航任务成功完成!" << std::endl;
           } else {
               std::cout << "导航任务失败, errorStatus: " << static_cast<int>(navigationResult.errorStatus) << std::endl;
           }
           taskCompleted = true;
       });

       // 等待任务开始执行
       std::this_thread::sleep_for(std::chrono::seconds(2));

       // 查询任务状态
       std::cout << "查询任务状态..." << std::endl;
       auto taskStatus = sdk.request1007_NavTaskStatus();
       std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;

       // 等待任务完成或用户输入
       std::cout << "按回车键取消任务，或等待任务完成..." << std::endl;
       std::string input;
       std::getline(std::cin, input);

       if (!taskCompleted) {
           // 取消导航任务
           std::cout << "取消导航任务..." << std::endl;
           if (sdk.request1004_CancelNavTask()) {
               std::cout << "导航任务已取消" << std::endl;
           } else {
               std::cerr << "导航任务取消失败" << std::endl;
           }
       }

       // 断开连接
       std::cout << "断开连接..." << std::endl;
       sdk.disconnect();
       std::cout << "已断开连接" << std::endl;

       return 0;
   }

异步操作示例
----------

以下示例展示了如何使用 SDK 的异步操作功能：

.. code-block:: cpp

   #include <iostream>
   #include <robotserver_sdk.h>
   #include <vector>
   #include <thread>
   #include <chrono>
   #include <atomic>
   #include <condition_variable>
   #include <mutex>

   using namespace robotserver_sdk;

   int main(int argc, char* argv[]) {
       if (argc < 3) {
           std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
           return 1;
       }

       std::string host = argv[1];
       int port = std::stoi(argv[2]);

       // 创建 SDK 实例
       RobotServerSdk sdk;

       // 连接到机器狗控制系统
       std::cout << "正在连接到 " << host << ":" << port << "..." << std::endl;
       if (!sdk.connect(host, port)) {
           std::cerr << "连接失败!" << std::endl;
           return 1;
       }
       std::cout << "连接成功!" << std::endl;

       // 创建导航点
       std::vector<NavigationPoint> points;
       NavigationPoint point;
       point.mapId = 1;
       point.value = 1;
       point.posX = 10.0;
       point.posY = 20.0;
       points.push_back(point);

       // 同步变量
       std::mutex mtx;
       std::condition_variable cv;
       bool taskFinished = false;
       NavigationResult result;

       // 发送导航任务
       std::cout << "发送导航任务..." << std::endl;
       sdk.request1003_StartNavTask(points, [&](const NavigationResult& navigationResult) {
           std::lock_guard<std::mutex> lock(mtx);
           result = navigationResult;
           taskFinished = true;
           cv.notify_one();
       });

       // 等待任务完成
       {
           std::unique_lock<std::mutex> lock(mtx);
           if (!cv.wait_for(lock, std::chrono::seconds(30), [&]{ return taskFinished; })) {
               std::cout << "任务超时!" << std::endl;
               sdk.request1004_CancelNavTask();
           } else {
               if (result.errorCode == ErrorCode_Navigation::SUCCESS) {
                   std::cout << "导航任务成功完成!" << std::endl;
               } else {
                   std::cout << "导航任务失败, errorStatus: " << static_cast<int>(result.errorStatus) << std::endl;
               }
           }
       }

       // 断开连接
       sdk.disconnect();

       return 0;
   }

错误处理示例
----------

以下示例展示了如何处理 SDK 操作中的错误：

.. code-block:: cpp

   #include <iostream>
   #include <robotserver_sdk.h>

   using namespace robotserver_sdk;

   void handleError(ErrorCode_Navigation errorCode, ErrorStatus errorStatus) {
       switch (errorCode) {
           case ErrorCode_Navigation::SUCCESS:
               std::cout << "操作成功" << std::endl;
               break;
           case ErrorCode_Navigation::FAILURE:
               std::cout << "操作失败，错误状态: " << static_cast<int>(errorStatus) << std::endl;
               // 根据错误状态码处理具体错误
               switch (errorStatus) {
                   case ErrorStatus::LOW_POWER_FAILED:
                       std::cout << "电量过低，无法执行任务" << std::endl;
                       break;
                   case ErrorStatus::NAVIGATION_PROCESS_NOT_STARTED_FAILED:
                       std::cout << "导航进程未启动，无法下发任务" << std::endl;
                       break;
                   // 处理其他错误状态...
                   default:
                       std::cout << "未知错误状态" << std::endl;
                       break;
               }
               break;
           case ErrorCode_Navigation::CANCELLED:
               std::cout << "操作被取消" << std::endl;
               break;
           case ErrorCode_Navigation::INVALID_PARAM:
               std::cout << "无效参数" << std::endl;
               break;
           case ErrorCode_Navigation::NOT_CONNECTED:
               std::cout << "未连接到机器狗控制系统" << std::endl;
               break;
           default:
               std::cout << "未知错误码" << std::endl;
               break;
       }
   }

   int main() {
       // 创建 SDK 实例
       RobotServerSdk sdk;

       // 尝试获取实时状态（未连接状态）
       auto status = sdk.request1002_RunTimeStatus();
       if (status.errorCode != ErrorCode_RealTimeStatus::SUCCESS) {
           std::cout << "获取实时状态失败，错误码: " << static_cast<int>(status.errorCode) << std::endl;
       }

       // 尝试连接
       if (!sdk.connect("192.168.1.106", 30000)) {
           std::cout << "连接失败" << std::endl;
           return 1;
       }

       // 发送导航任务并处理错误
       std::vector<NavigationPoint> points;
       // ... 初始化导航点 ...

       sdk.request1003_StartNavTask(points, [](const NavigationResult& result) {
           handleError(result.errorCode, result.errorStatus);
       });

       // 断开连接
       sdk.disconnect();

       return 0;
   }

更多示例
-------

更多示例可以在 ``examples`` 目录下找到：

- ``examples/basic/basic_example.cpp``: 基础使用示例
- ``examples/async/async_example.cpp``: 异步操作示例
- ``examples/error_handling/error_handling_example.cpp``: 错误处理示例
