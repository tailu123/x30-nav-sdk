# X30 机器狗RobotServer SDK 架构设计

## 1. 架构概述

X30 机器狗RobotServer SDK 采用**简洁的分层架构**设计，确保系统具有**高可维护性**和**良好扩展性**。本文档简要介绍 SDK 的整体架构和核心组件。

### 1.1 核心层次结构

SDK 由以下三个主要层次组成：

| 层次 | 职责 | 关键组件 |
|------|------|---------|
| **应用层** | 负责调用接口层提供的接口，集成实现业务逻辑 | ___ |
| **接口层** | 负责请求/响应的管理，对外提供功能接口 | RobotServerSdk 类、RobotServerSdkImpl 类 |
| **通信层** | 负责协议序列化，数据传输 | AsioNetworkModel 类、Serializer 类 |

### 1.2 架构图示

```
┌────────────────────────────────────────────────────────────────────────┐
│                          【应用层（Application）】                        │
│                主要功能：负责调用接口层提供的接口，集成实现业务逻辑             │
└────────────────────────────────┬───────────────────────────────────────┘
                                 │
                                 ▼
┌────────────────────────────────────────────────────────────────────────┐
│                         【接口层（Interface Layer）】                     │
│               主要功能：负责请求/响应的管理，对外提供功能接口                  │
│               关键类：RobotServerSdk 类，RobotServerSdkImpl 类           │
└────────────────────────────────┬───────────────────────────────────────┘
                                 │
                                 ▼
┌────────────────────────────────────────────────────────────────────────┐
│                      【通信层（Communication Layer）】                    │
│                    主要功能：负责协议序列化，数据传输                        │
│   关键类：Serializer 类（协议序列化/反序列化），AsioNetworkModel 类（网络通信） │
└────────────────────────────────────────────────────────────────────────┘
```

## 2. 组件详解

### 2.1 接口层（Interface Layer）

接口层是 SDK 与用户应用程序交互的桥梁，提供简洁易用的 API，同时负责业务逻辑实现和各组件协调。

#### 核心组件

- **RobotServerSdk 类**：SDK 的主入口点，封装所有功能接口
- **RobotServerSdkImpl 类**：RobotServerSdk 的具体实现
- **类型定义**：包括数据结构、枚举类型和回调函数类型
- **网络回调接口实现**：处理网络消息
- **请求响应管理**：跟踪请求和响应的对应关系

#### 代码位置

- `include/robotserver_sdk.h`：定义 RobotServerSdk 类及其方法
- `include/types.h`：定义各种数据结构和类型
- `src/robotserver_sdk.cpp`：实现 RobotServerSdkImpl 类

#### 设计特点

- 采用 **PIMPL 模式**（指针实现）隐藏实现细节
- 提供**同步和异步**两种操作方式
- 使用**回调机制**处理异步事件
- 实现 **INetworkCallback 接口**接收网络消息
- 使用**条件变量**和**互斥锁**确保线程安全
- 管理**请求超时**和**错误处理**

### 2.2 通信层（Communication Layer）

通信层负责与机器狗控制系统进行通信，包括网络连接管理和协议处理。

#### 核心组件

- **Serializer 类**：处理消息的序列化和反序列化
- **AsioNetworkModel 类**：基于 Boost.Asio 的网络实现
- **INetworkCallback 接口**：定义网络层回调接口

#### 代码位置

- `src/protocol/serializer.hpp/cpp`：协议处理实现
- `src/network/asio_network_model.hpp/cpp`：网络通信实现

#### 设计特点

- 基于 **Boost.Asio** 实现**异步 TCP 通信**
- 使用 **Strand** 确保回调的线程安全
- 支持 **XML** 协议格式

## 3. 类图与关系

下面的类图展示了 SDK 主要组件之间的关系：

```mermaid
classDiagram
    %% 接口层
    class RobotServerSdk {
        -RobotServerSdkImpl* impl_
        +connect(host: string, port: uint16_t)
        +disconnect()
        +isConnected()
        +request1002_RunTimeStatus()
        +request1003_StartNavTask(points: vector<NavigationPoint>, callback: NavigationResultCallback)
        +request1004_CancelNavTask()
        +request1007_NavTaskStatus()
    }

    class RobotServerSdkImpl {
        -network_model_: unique_ptr<AsioNetworkModel>
        +onMessageReceived(message: unique_ptr<IMessage>) // 处理响应，支持请求与响应匹配
        -generateSequenceNumber()
    }

    %% 通信层
    class INetworkCallback {
        <<interface>>
        +onMessageReceived(message: unique_ptr<IMessage>)*
    }

    class AsioNetworkModel {
        -io_context_: boost::asio::io_context
        -socket_: boost::asio::ip::tcp::socket
        -strand_: boost::asio::io_context::strand
        -io_thread_: std::thread
        -connected_: atomic<bool>
        +connect(host: string, port: uint16_t)
        +disconnect()
        +sendMessage(message: IMessage)
        -receive()
        -send()
    }

    class Serializer {
        +serializeMessage(message: IMessage)
        +deserializeMessage(data: string)
    }

    %% 关系定义
    RobotServerSdk *-- RobotServerSdkImpl : 包含
    RobotServerSdkImpl ..|> INetworkCallback : 实现
    RobotServerSdkImpl *-- AsioNetworkModel : 使用
    AsioNetworkModel ..> Serializer : 依赖
```

## 4. 数据流

SDK 中的数据流展示了请求和响应的完整生命周期。

### 4.1 请求流程

1. **用户调用** → 用户通过 RobotServerSdk 接口发起请求
2. **请求转发** → RobotServerSdkImpl 接收并处理请求
3. **消息创建** → 创建对应的请求消息对象
4. **消息序列化** → Serializer 将消息序列化为二进制数据
5. **网络发送** → AsioNetworkModel 通过网络发送数据

### 4.2 响应流程

1. **数据接收** → AsioNetworkModel 接收网络数据
2. **数据解析** → Serializer 解析数据并创建响应消息
3. **消息处理** → RobotServerSdkImpl 处理响应消息
4. **结果返回** → 结果通过同步返回或异步回调传递给用户

### 4.3 时序图

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant SDK as RobotServerSdk
    participant Impl as RobotServerSdkImpl
    participant Proto as Serializer
    participant Net as AsioNetworkModel
    participant Dog as 机器狗系统

    App->>SDK: 调用API
    SDK->>Impl: 转发请求
    Impl->>Proto: 创建请求消息
    Proto->>Proto: 序列化消息
    Proto->>Net: 传递序列化数据
    Net->>Dog: 发送网络数据

    Dog-->>Net: 返回响应数据
    Net-->>Proto: 传递原始数据
    Proto-->>Proto: 解析响应数据
    Proto-->>Impl: 创建响应消息
    Impl-->>SDK: 处理响应
    SDK-->>App: 返回结果
```

### 4.4 connect 流程

实线箭头表示用户线程; 连接时期虚线箭头依赖系统底层IO; 收发数据时虚线箭头表示IO线程


```mermaid
sequenceDiagram
    participant App as 应用程序
    participant SDK as RobotServerSdk
    participant Impl as RobotServerSdkImpl
    participant Net as AsioNetworkModel
    participant IOThread as IO线程
    participant Dog as X30机器狗系统

    %% Connection Flow
    App->>SDK: connect(host, port)
    SDK->>Impl: connect(host, port)
    Impl->>Net: connect(host, port)
    
    %% 异步连接请求
    Net->>Dog: async_connect(socket_, endpoints, callback)
    
    %% 主线程等待连接完成或超时
    Net->>Net: run_one_for(connection_timeout_)
    
    Net-->>Dog: TCP连接请求
    Dog-->>Net: 连接响应
    Net-->>Net: 调用连接完成回调,connected_ = true
    
    %% 启动专用IO线程
    Net->>IOThread: 创建并启动线程(ioThreadFunc)
    Note over IOThread,IOThread: IO线程持续运行io_context_.run()
    
    %% 启动第一次接收
    Net->>Net: startReceive()
    
    %% 返回连接结果
    Net->>Impl: 返回true(连接成功)
    Impl->>SDK: 返回true
    SDK->>App: 返回true
    
    %% 后续的异步操作
    IOThread-->>Dog: send
    Dog-->>IOThread: receive
```

### 4.5 请求流程(同步) 1002, 1004, 1007

实线箭头表示用户线程，虚线箭头表示IO线程

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant SDK as RobotServerSdk
    participant Impl as RobotServerSdkImpl
    participant Proto as Serializer
    participant Net as AsioNetworkModel
    participant IO as IO线程
    participant Dog as X30机器狗系统

    %% Request Flow
    App->>SDK: request1002_RunTimeStatus()
    SDK->>Impl: request1002_RunTimeStatus()
    Impl->>Impl: generateSequenceNumber()
    Impl->>Proto: sendMessage()
    Proto->>Net: serializeMessage()
    Net->>IO: 将async_write请求放入io_context队列
    Net->>Impl: return
    Impl->>Impl: wait_for(request_timeout) 等待响应
    IO-->>Dog: 执行boost::asio::async_write()

    %% Response Flow
    Dog-->>IO: 返回响应数据
    IO-->>Net: 执行async_read_some回调
    Net-->>Proto: deserializeMessage(data)
    Proto-->>Impl: onMessageReceived(message)
    Impl-->>Impl: 请求与响应匹配
    Impl-->>Impl: notify_one唤醒wait_for
    Impl->>SDK: 响应处理转换为RealTimeStatus并返回
    SDK->>App: 返回结果
```

### 4.6 请求流程(异步) 1003

实线箭头表示用户线程，虚线箭头表示IO线程

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant SDK as RobotServerSdk
    participant Impl as RobotServerSdkImpl
    participant Proto as Serializer
    participant Net as AsioNetworkModel
    participant IO as IO线程
    participant Dog as X30机器狗系统

    %% Request Flow
    App->>SDK: request1003_StartNavTask(points, callback)
    SDK->>Impl: request1003_StartNavTask(points, callback)
    Impl->>Impl: generateSequenceNumber()
    Impl->>Impl: 保存回调函数 [seqNum, callback]
    Impl->>Proto: sendMessage()
    Proto->>Net: serializeMessage()
    Net->>IO: 将async_write请求放入io_context队列
    Net->>Impl: return
    Impl->>SDK: return
    SDK->>App: return
    IO-->>Dog: 执行boost::asio::async_write()

    %% Response Flow
    Dog-->>IO: 返回响应数据
    IO-->>Net: 执行async_read_some回调
    Net-->>Proto: deserializeMessage(data)
    Proto-->>Impl: onMessageReceived(message)
    Impl-->>Impl: 请求与响应匹配, 找到回调函数 [seqNum, callback]
    Impl-->>App: 通过回调函数返回结果
```

## 5. 设计特点与优势

### 5.1 核心设计特点

| 特点 | 描述 | 优势 |
|------|------|------|
| **分层设计** | 清晰的三层架构 | 降低耦合，提高可维护性 |
| **接口分离** | 通过接口定义组件交互 | 便于单元测试和模块替换 |
| **异步处理** | 支持同步和异步操作 | 提高系统灵活性和性能 |
| **线程安全** | 多种线程同步机制 | 确保多线程环境安全 |

### 5.2 扩展性设计

SDK 架构设计考虑了未来扩展需求：

1. **新网络协议支持**
   - 实现新的网络模型类
   - 或扩展现有协议处理类

2. **新序列化格式支持**
   - 扩展 Serializer 类
   - 或实现新的协议处理类

## 6. 总结

X30 机器狗导航 SDK 采用简洁的三层架构设计，各组件职责明确，相互独立，具有以下优势：

- **高可维护性**：清晰的层次结构和接口定义
- **良好扩展性**：松耦合设计便于添加新功能
- **易用性**：简洁的 API 设计，支持同步和异步操作

通过这种架构设计，SDK 为开发者提供了稳定、可靠、易用的机器狗导航控制功能，同时保持了系统的灵活性和可扩展性。

## 下一步

- 查看 [快速开始](quick_start.md) 了解 SDK 的整体架构和设计理念
- 查看 [API 参考](api_reference.md) 了解更多 SDK 功能
