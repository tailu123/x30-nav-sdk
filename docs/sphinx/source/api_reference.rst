API 参考
========

本文档提供机器狗 RobotServer SDK 的详细 API 参考，包括所有公共类、方法、枚举和数据结构的说明。

RobotServerSdk 类
----------------

.. doxygenclass:: robotserver_sdk::RobotServerSdk
   :members:
   :protected-members:
   :private-members:
   :undoc-members:

构造函数和析构函数
^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 创建 RobotServerSdk 实例
    */
   RobotServerSdk();

   /**
    * @brief 销毁 RobotServerSdk 实例，释放所有资源
    */
   ~RobotServerSdk();

连接管理
^^^^^^^

.. code-block:: cpp

   /**
    * @brief 连接到机器狗控制系统
    * @param host 主机地址（IP 或域名）
    * @param port 端口号
    * @return 如果连接请求已成功发送，则返回 true；否则返回 false
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

实时状态
^^^^^^^

.. code-block:: cpp

   /**
    * @brief 获取机器狗实时状态（同步方法）
    * @return 包含实时状态如位置、速度、角度、电量等
    */
   RealTimeStatus request1002_RunTimeStatus();

导航任务
^^^^^^^

.. code-block:: cpp

   /**
    * @brief 开始导航任务（异步方法）
    * @param navigation_points 导航点列表
    * @param navigationResultCallback 结果回调函数
    * @note 导航任务完成后，会通过回调函数返回结果; 回调函数在IO线程中调用，不应执行长时间操作
    */
   void request1003_StartNavTask(
       const std::vector<NavigationPoint>& navigation_points,
       NavigationResultCallback navigationResultCallback);

   /**
    * @brief 取消导航任务（同步方法）
    * @return 如果取消成功，则返回 true；否则返回 false
    */
   bool request1004_CancelNavTask();

   /**
    * @brief 查询导航任务状态（同步方法）
    * @return 包含任务状态和错误码
    */
   TaskStatusResult request1007_NavTaskStatus();

版本信息
^^^^^^^

.. code-block:: cpp

   /**
    * @brief 获取 SDK 版本信息
    * @return SDK 版本字符串
    */
   static std::string getVersion();

数据类型
-------

NavigationPoint
^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1003 导航点
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

RealTimeStatus
^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1002 获取机器狗实时状态
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

NavigationResult
^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1003 导航任务结果
    */
   struct NavigationResult {
       int value = 0;                                                  ///< 导航任务目标点编号，与下发导航任务请求对应
       ErrorCode_Navigation errorCode = ErrorCode_Navigation::SUCCESS; ///< 错误码 0:成功; 1:失败; 2:取消
       ErrorStatus errorStatus = ErrorStatus::DEFAULT;                 ///< 错误状态码; 导航任务失败的具体原因
   };

TaskStatusResult
^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1007 任务状态查询结果
    */
   struct TaskStatusResult {
       int value = 0;                                                      ///< 导航任务目标点编号，与下发导航任务请求对应
       NavigationStatus status = NavigationStatus::COMPLETED;              ///< 导航状态:  0:已完成; 1:执行中; 2:失败
       ErrorCode_QueryStatus errorCode = ErrorCode_QueryStatus::COMPLETED; ///< 错误码:   0:成功; 1:执行中; 2:失败
   };

枚举类型
-------

ErrorCode_Navigation
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1003 导航任务响应ErrorCode枚举
    */
   enum class ErrorCode_Navigation {
       SUCCESS = 0,      ///< 操作成功
       FAILURE = 1,      ///< 操作失败
       CANCELLED = 2,    ///< 操作被取消

       INVALID_PARAM = 3,///< 无效参数
       NOT_CONNECTED = 4 ///< 未连接
   };

ErrorCode_QueryStatus
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1007 任务状态查询ErrorCode枚举
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

Status_QueryStatus
^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1007 任务状态查询status枚举
    */
   enum class Status_QueryStatus {
       COMPLETED = 0,    ///< 任务已完成
       EXECUTING = 1,    ///< 任务执行中
       FAILED = -1       ///< 无法执行
   };

ErrorCode_RealTimeStatus
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 1002 实时状态查询ErrorCode枚举
    */
   enum class ErrorCode_RealTimeStatus {
       SUCCESS = 0,            ///< 操作成功

       INVALID_RESPONSE = 1,   ///< 无效响应
       TIMEOUT = 2,            ///< 超时
       NOT_CONNECTED = 3,      ///< 未连接
       UNKNOWN_ERROR = 4       ///< 未知错误
   };

MessageType
^^^^^^^^^^

.. code-block:: cpp

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

回调函数
-------

NavigationResultCallback
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   /**
    * @brief 导航结果回调函数类型
    * @param result 导航结果
    */
   using NavigationResultCallback = std::function<void(const NavigationResult& result)>;

错误处理
-------

SDK 使用 ``ErrorCode`` 枚举表示操作结果。每个同步方法都返回一个包含结果和错误码的结构体，每个异步方法都在回调函数中提供结果和错误码。

线程安全性
--------

SDK 的所有公共 API 都是线程安全的，可以从多个线程同时调用。回调函数在 IO 线程中执行，不应执行长时间操作。

版本历史
-------

.. list-table::
   :header-rows: 1
   :widths: 20 30 50

   * - 版本
     - 发布日期
     - 主要变更
   * - 0.1.0
     - 2025-03-07
     - 初始版本
