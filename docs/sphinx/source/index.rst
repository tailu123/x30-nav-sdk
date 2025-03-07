RobotServer SDK 文档
====================

欢迎使用 RobotServer SDK 文档！本文档提供了关于如何使用 RobotServer SDK 控制和监控机器狗导航任务的详细信息。

.. toctree::
   :maxdepth: 2
   :caption: 目录:

   introduction
   quick_start
   architecture
   api_reference
   examples
   faq

简介
----

RobotServer SDK 提供了一套简单易用的接口，用于控制和监控机器狗的导航任务。该 SDK 封装了底层的协议和网络通信细节，使开发者能够专注于业务逻辑的实现。

主要功能
--------

- 连接和断开与机器狗控制系统的通信
- 获取机器狗的实时状态信息
- 发送导航任务指令
- 取消正在执行的导航任务
- 查询当前导航任务的执行状态

系统要求
--------

- C++17 或更高版本
- CMake 3.10 或更高版本
- Boost 1.66 或更高版本 (用于网络通信)
- nlohmann/json (用于JSON解析)
- rapidxml (用于XML解析)

索引和表格
----------

* :ref:`genindex`
* :ref:`search`
