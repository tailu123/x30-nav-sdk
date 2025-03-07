常见问题解答
============

本文档回答了关于 RobotServer SDK 的一些常见问题。

一般问题
-------

RobotServer SDK 是什么？
^^^^^^^^^^^^^^^^^^^^^

RobotServer SDK 是一个用于控制和监控机器狗导航任务的 C++ 库。它提供了一套简单易用的 API，使开发者能够轻松地与机器狗控制系统进行通信，发送导航指令，并获取实时状态信息。

RobotServer SDK 支持哪些操作系统？
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

RobotServer SDK 支持以下操作系统：

- Linux (Ubuntu 18.04 及以上)
- macOS (10.14 及以上)
- Windows (Windows 10 及以上)

RobotServer SDK 的许可证是什么？
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

RobotServer SDK 采用 MIT 许可证。详情请参阅项目根目录下的 LICENSE 文件。

安装问题
-------

如何安装 RobotServer SDK？
^^^^^^^^^^^^^^^^^^^^^^^

您可以通过以下方式安装 RobotServer SDK：

1. 从源代码构建：

.. code-block:: bash

   git clone https://github.com/tailu123/robotserver-sdk.git
   cd robotserver_sdk
   mkdir build && cd build
   cmake ..
   make
   sudo make install

2. 使用预编译的二进制包（如果可用）：

.. code-block:: bash

   # Ubuntu/Debian
   sudo apt-get install robotserver-sdk

   # macOS
   brew install robotserver-sdk

   # Windows
   # 下载并运行安装程序

安装依赖时遇到问题怎么办？
^^^^^^^^^^^^^^^^^^^^^

如果您在安装依赖时遇到问题，请尝试以下解决方案：

1. 确保您的系统已更新：

.. code-block:: bash

   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get upgrade

   # macOS
   brew update
   brew upgrade

2. 手动安装依赖：

.. code-block:: bash

   # Ubuntu/Debian
   sudo apt-get install libboost-all-dev nlohmann-json3-dev

   # macOS
   brew install boost nlohmann-json

3. 如果问题仍然存在，请查看项目 GitHub 页面上的 Issues 部分，或提交新的 Issue。

使用问题
-------

如何连接到机器狗控制系统？
^^^^^^^^^^^^^^^^^^^^^

使用 ``connect`` 方法连接到机器狗控制系统：

.. code-block:: cpp

   RobotServerSdk sdk;
   if (!sdk.connect("192.168.1.106", 30000)) {
       std::cerr << "连接失败!" << std::endl;
       return 1;
   }

连接失败的常见原因有哪些？
^^^^^^^^^^^^^^^^^^^^^

连接失败的常见原因包括：

1. 网络连接问题：确保您的计算机和机器狗控制系统在同一网络中，并且可以相互通信。
2. IP 地址或端口错误：确保您提供了正确的 IP 地址和端口号。
3. 机器狗控制系统未运行：确保机器狗控制系统已启动并正在运行。
4. 防火墙阻止：检查防火墙设置，确保允许通信。

如何处理导航任务中的错误？
^^^^^^^^^^^^^^^^^^^^^

导航任务中的错误可以通过回调函数中的 ``errorCode`` 和 ``errorStatus`` 参数来处理：

.. code-block:: cpp

   sdk.request1003_StartNavTask(points, [](const NavigationResult& result) {
       if (result.errorCode == ErrorCode_Navigation::SUCCESS) {
           std::cout << "导航任务成功完成!" << std::endl;
       } else {
           std::cout << "导航任务失败, errorCode: " << static_cast<int>(result.errorCode)
                     << ", errorStatus: " << static_cast<int>(result.errorStatus) << std::endl;

           // 根据错误码和错误状态码处理具体错误
           // ...
       }
   });

如何在多线程环境中使用 SDK？
^^^^^^^^^^^^^^^^^^^^^^^^

RobotServer SDK 的所有公共 API 都是线程安全的，可以从多个线程同时调用。但是，回调函数在 IO 线程中执行，不应执行长时间操作。如果需要在回调函数中执行长时间操作，应该将操作放在单独的线程中执行：

.. code-block:: cpp

   sdk.request1003_StartNavTask(points, [](const NavigationResult& result) {
       // 在单独的线程中处理结果
       std::thread([result]() {
           // 处理结果
           // 可以执行长时间操作
       }).detach();
   });

性能问题
-------

SDK 的性能如何？
^^^^^^^^^^^^^

RobotServer SDK 经过优化，具有高性能和低延迟。在正常网络条件下，请求和响应的往返时间通常在几毫秒到几十毫秒之间。

如何提高 SDK 的性能？
^^^^^^^^^^^^^^^^^

要提高 SDK 的性能，可以考虑以下几点：

1. 使用异步操作：对于不需要立即结果的操作，使用异步方法可以提高性能。
2. 减少请求频率：避免频繁发送请求，特别是获取实时状态的请求。
3. 优化网络环境：确保网络连接稳定，延迟低。
4. 使用合适的编译选项：使用优化编译选项构建 SDK 和应用程序。

故障排除
-------

如何调试 SDK 问题？
^^^^^^^^^^^^^^^

调试 SDK 问题的方法包括：

1. 启用日志：SDK 提供了详细的日志功能，可以帮助诊断问题。
2. 使用调试构建：使用调试模式构建 SDK 和应用程序。
3. 检查错误码和错误状态码：SDK 的每个操作都会返回错误码和错误状态码，可以帮助诊断问题。
4. 使用网络分析工具：使用 Wireshark 等工具分析网络通信。

常见错误码及其含义是什么？
^^^^^^^^^^^^^^^^^^^^^

常见错误码及其含义包括：

- ``ErrorCode_Navigation::SUCCESS (0)``：操作成功。
- ``ErrorCode_Navigation::FAILURE (1)``：操作失败，查看 ``errorStatus`` 获取详细信息。
- ``ErrorCode_Navigation::CANCELLED (2)``：操作被取消。
- ``ErrorCode_Navigation::INVALID_PARAM (3)``：无效参数。
- ``ErrorCode_Navigation::NOT_CONNECTED (4)``：未连接到机器狗控制系统。

更多错误码及其含义，请参阅 :doc:`api_reference` 中的枚举类型部分。

如何获取更多帮助？
^^^^^^^^^^^^^^^

如果您在使用 RobotServer SDK 时遇到问题，可以通过以下方式获取帮助：

1. 查阅文档：本文档提供了详细的 API 参考和使用示例。
2. 查看示例代码：``examples`` 目录下的示例代码展示了 SDK 的各种用法。
3. 提交 Issue：在项目 GitHub 页面上提交 Issue。
4. 联系支持团队：发送邮件至 support@example.com。
