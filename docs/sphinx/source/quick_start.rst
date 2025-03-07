快速入门
========

本指南将帮助您快速上手使用 RobotServer SDK，即使您不熟悉 C++ 编程语言。

系统要求
--------

在开始使用 SDK 之前，请确保您的系统满足以下要求：

- 操作系统：Linux、macOS 或 Windows
- C++17 或更高版本的编译器
- CMake 3.10 或更高版本
- Boost 1.66 或更高版本
- nlohmann/json 库
- rapidxml 库

安装依赖
--------

您可以使用提供的脚本安装所有依赖：

.. code-block:: bash

   chmod +x scripts/install_dependencies.sh
   ./scripts/install_dependencies.sh

安装 SDK
--------

从源代码构建
^^^^^^^^^^^

1. 克隆 SDK 代码库：

.. code-block:: bash

   git clone https://github.com/tailu123/robotserver-sdk.git
   cd robotserver_sdk

2. 创建构建目录并编译：

.. code-block:: bash

   mkdir build && cd build
   cmake ..
   make

3. 安装 SDK（可选）：

.. code-block:: bash

   sudo make install

基本用法
--------

完整示例
^^^^^^^^

参考 ``examples/basic/basic_example.cpp`` 文件，实现了一个简单的示例，展示如何使用 SDK 连接到机器狗并发送导航任务。

1. 包含头文件
^^^^^^^^^^^^^

.. code-block:: cpp

   #include <robotserver_sdk.h>

2. 创建 SDK 实例
^^^^^^^^^^^^^^^

.. code-block:: cpp

   // 创建 SDK 实例
   robotserver_sdk::RobotServerSdk sdk;

3. 连接到机器狗控制系统
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   // 连接到机器狗控制系统
   if (!sdk.connect("192.168.1.106", 30000)) {
       std::cerr << "连接失败!" << std::endl;
       return 1;
   }

4. 获取实时状态
^^^^^^^^^^^^^

.. code-block:: cpp

   // 获取实时状态
   auto status = sdk.request1002_RunTimeStatus();
   std::cout << "当前位置: (" << status.posX << ", " << status.posY << ", " << status.posZ << ")" << std::endl;

5. 发送导航任务
^^^^^^^^^^^^^

.. code-block:: cpp

   // 创建导航点
   auto points = default_navigation_points;

   // 发送导航任务
   sdk.request1003_StartNavTask(points, [](const NavigationResult& navigationResult) {
       if (navigationResult.errorCode == ErrorCode_Navigation::SUCCESS) {
           std::cout << "导航任务成功完成!" << std::endl;
       } else {
           std::cout << "导航任务失败, errorStatus: " << static_cast<int>(navigationResult.errorStatus) << std::endl;
       }
   });

6. 查询导航任务状态
^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   // 查询任务状态
   auto taskStatus = sdk.request1007_NavTaskStatus();
   std::cout << "任务状态: " << static_cast<int>(taskStatus.status) << std::endl;

7. 取消导航任务
^^^^^^^^^^^^^

.. code-block:: cpp

   // 取消导航任务
   if (sdk.request1004_CancelNavTask()) {
       std::cout << "导航任务已取消" << std::endl;
   } else {
       std::cerr << "导航任务取消失败" << std::endl;
   }

8. 断开连接
^^^^^^^^^^

.. code-block:: cpp

   // 断开连接
   sdk.disconnect();

下一步
------

- 查看 :doc:`architecture` 了解 SDK 的整体架构和设计理念
- 查看 :doc:`api_reference` 了解更多 SDK 功能
