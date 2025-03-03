# 网络通信模型

X30 机器狗导航 SDK 使用基于 TCP/IP 的网络通信模型与机器狗控制系统进行通信。本文档详细介绍 SDK 的网络通信架构、协议设计、消息格式以及错误处理机制，帮助开发者理解 SDK 的网络通信工作原理。

## 网络架构概述

SDK 的网络通信架构采用客户端-服务器模型：

- **客户端**：SDK 作为客户端，连接到机器狗控制系统
- **服务器**：机器狗控制系统作为服务器，接收并处理 SDK 发送的请求

通信基于 TCP/IP 协议，确保可靠的数据传输。SDK 使用 Boost.Asio 库实现异步网络通信，提供高效的 I/O 操作和事件驱动的编程模型。

## 网络层组件

SDK 的网络层主要由以下组件组成：

### 1. 网络模型接口（BaseNetworkModel）

`BaseNetworkModel` 是一个抽象接口，定义了网络通信的基本操作：

```cpp
class BaseNetworkModel {
public:
    virtual ~BaseNetworkModel() = default;

    // 连接到指定主机和端口
    virtual bool connect(const std::string& host, uint16_t port) = 0;

    // 断开连接
    virtual void disconnect() = 0;

    // 检查是否已连接
    virtual bool isConnected() const = 0;

    // 发送消息
    virtual bool sendMessage(const std::shared_ptr<IMessage>& message) = 0;

    // 设置消息接收回调
    virtual void setMessageCallback(MessageCallback callback) = 0;

    // 设置连接状态变化回调
    virtual void setConnectionCallback(ConnectionCallback callback) = 0;
};
```

### 2. ASIO 网络模型实现（AsioNetworkModel）

`AsioNetworkModel` 是基于 Boost.Asio 的网络模型实现，提供异步 TCP 通信功能：

```cpp
class AsioNetworkModel : public BaseNetworkModel {
public:
    AsioNetworkModel();
    ~AsioNetworkModel() override;

    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool isConnected() const override;
    bool sendMessage(const std::shared_ptr<IMessage>& message) override;
    void setMessageCallback(MessageCallback callback) override;
    void setConnectionCallback(ConnectionCallback callback) override;

private:
    // Boost.Asio 组件
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
    boost::asio::io_context::strand strand_;
    std::thread io_thread_;

    // 接收缓冲区
    std::vector<char> receive_buffer_;

    // 回调函数
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;

    // 状态标志
    std::atomic<bool> connected_{false};
    std::atomic<bool> stopping_{false};

    // 私有方法
    void startReceive();
    void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void notifyConnectionStatus(bool connected, const std::string& error_message = "");
};
```

### 3. 协议处理器（X30Protocol）

`X30Protocol` 负责处理网络协议，包括消息的序列化和反序列化：

```cpp
class X30Protocol {
public:
    X30Protocol();
    ~X30Protocol();

    // 解析接收到的数据
    std::vector<std::shared_ptr<IMessage>> parseReceivedData(const std::vector<char>& data, size_t bytes_transferred);

    // 序列化消息
    std::string serializeMessage(const std::shared_ptr<IMessage>& message);

private:
    // 提取消息类型
    MessageType extractMessageType(const std::string& xml_data);

    // 处理 XML 数据
    std::shared_ptr<IMessage> handleXmlData(const std::string& xml_data, MessageType type);
};
```

## 通信流程

SDK 的网络通信流程如下：

### 1. 连接建立

```
+-------------+                  +------------------+
|    SDK      |                  | 机器狗控制系统    |
+-------------+                  +------------------+
      |                                  |
      | 1. connect(host, port)           |
      |--------------------------------->|
      |                                  |
      | 2. TCP 连接建立                   |
      |<---------------------------------|
      |                                  |
      | 3. 触发 CONNECTION_ESTABLISHED 事件|
      |                                  |
```

### 2. 消息发送

```
+-------------+                  +------------------+
|    SDK      |                  | 机器狗控制系统    |
+-------------+                  +------------------+
      |                                  |
      | 1. 创建消息对象                   |
      |                                  |
      | 2. sendMessage(message)          |
      |--------------------------------->|
      |                                  |
      | 3. 序列化消息                     |
      |                                  |
      | 4. 发送序列化后的数据              |
      |--------------------------------->|
      |                                  |
```

### 3. 消息接收

```
+-------------+                  +------------------+
|    SDK      |                  | 机器狗控制系统    |
+-------------+                  +------------------+
      |                                  |
      |                                  |
      |                                  |
      |<---------------------------------|
      | 1. 接收数据                       |
      |                                  |
      | 2. 解析协议头                     |
      |                                  |
      | 3. 反序列化消息                   |
      |                                  |
      | 4. 触发消息回调                   |
      |                                  |
```

### 4. 断开连接

```
+-------------+                  +------------------+
|    SDK      |                  | 机器狗控制系统    |
+-------------+                  +------------------+
      |                                  |
      | 1. disconnect()                  |
      |--------------------------------->|
      |                                  |
      | 2. 关闭 TCP 连接                  |
      |<---------------------------------|
      |                                  |
      | 3. 触发 CONNECTION_CLOSED 事件    |
      |                                  |
```

## 消息格式

SDK 支持两种消息格式：XML 和 JSON。默认使用 XML 格式进行通信。

### 1. 协议头

每个消息都包含一个协议头，用于标识消息的长度和类型：

```cpp
struct ProtocolHeader {
    uint32_t message_size;  // 消息体的大小（字节）
    uint16_t message_type;  // 消息类型
    uint16_t reserved;      // 保留字段，用于对齐
};
```

### 2. XML 消息格式

XML 消息的基本格式如下：

```xml
<message type="message_type">
    <param1>value1</param1>
    <param2>value2</param2>
    <!-- 其他参数 -->
</message>
```

例如，实时状态请求消息：

```xml
<message type="get_real_time_status">
    <request_id>12345</request_id>
</message>
```

### 3. JSON 消息格式

JSON 消息的基本格式如下：

```json
{
    "message_type": "message_type",
    "param1": "value1",
    "param2": "value2"
}
```

例如，实时状态请求消息：

```json
{
    "message_type": "get_real_time_status",
    "request_id": 12345
}
```

## 消息类型

SDK 支持以下消息类型：

| 消息类型 | 描述 | 方向 |
|---------|------|------|
| `get_real_time_status` | 获取机器狗实时状态 | SDK → 控制系统 |
| `real_time_status_response` | 实时状态响应 | 控制系统 → SDK |
| `start_navigation` | 开始导航任务 | SDK → 控制系统 |
| `start_navigation_response` | 导航任务响应 | 控制系统 → SDK |
| `cancel_navigation` | 取消导航任务 | SDK → 控制系统 |
| `cancel_navigation_response` | 取消导航任务响应 | 控制系统 → SDK |
| `query_task_status` | 查询任务状态 | SDK → 控制系统 |
| `task_status_response` | 任务状态响应 | 控制系统 → SDK |
| `event_notification` | 事件通知 | 控制系统 → SDK |

## 异步通信机制

SDK 使用 Boost.Asio 实现异步通信，主要特点包括：

### 1. 异步连接

```cpp
bool AsioNetworkModel::connect(const std::string& host, uint16_t port) {
    try {
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        boost::asio::async_connect(*socket_, endpoints,
            boost::asio::bind_executor(strand_,
                [this](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
                    if (!error) {
                        connected_ = true;
                        notifyConnectionStatus(true);
                        startReceive();
                    } else {
                        notifyConnectionStatus(false, error.message());
                    }
                }
            )
        );

        // 启动 IO 线程
        if (!io_thread_.joinable()) {
            io_thread_ = std::thread([this]() {
                io_context_.run();
            });
        }

        return true;
    } catch (const std::exception& e) {
        notifyConnectionStatus(false, e.what());
        return false;
    }
}
```

### 2. 异步接收

```cpp
void AsioNetworkModel::startReceive() {
    if (!socket_ || !socket_->is_open()) {
        return;
    }

    socket_->async_read_some(
        boost::asio::buffer(receive_buffer_),
        boost::asio::bind_executor(strand_,
            [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleReceive(error, bytes_transferred);
            }
        )
    );
}

void AsioNetworkModel::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        // 处理接收到的数据
        if (message_callback_) {
            auto protocol = std::make_shared<X30Protocol>();
            auto messages = protocol->parseReceivedData(receive_buffer_, bytes_transferred);

            for (const auto& message : messages) {
                message_callback_(message);
            }
        }

        // 继续接收数据
        startReceive();
    } else if (error != boost::asio::error::operation_aborted) {
        // 处理错误
        connected_ = false;
        notifyConnectionStatus(false, error.message());
    }
}
```

### 3. 异步发送

```cpp
bool AsioNetworkModel::sendMessage(const std::shared_ptr<IMessage>& message) {
    if (!socket_ || !socket_->is_open() || !connected_) {
        return false;
    }

    try {
        auto protocol = std::make_shared<X30Protocol>();
        std::string data = protocol->serializeMessage(message);

        boost::asio::async_write(*socket_,
            boost::asio::buffer(data),
            boost::asio::bind_executor(strand_,
                [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (error && error != boost::asio::error::operation_aborted) {
                        connected_ = false;
                        notifyConnectionStatus(false, error.message());
                    }
                }
            )
        );

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}
```

## 错误处理

SDK 的网络层实现了多种错误处理机制：

### 1. 连接错误

连接错误通过 `ConnectionCallback` 回调函数通知应用程序：

```cpp
void AsioNetworkModel::notifyConnectionStatus(bool connected, const std::string& error_message) {
    if (connection_callback_) {
        connection_callback_(connected, error_message);
    }
}
```

### 2. 发送错误

发送错误通过异步写入操作的回调函数处理：

```cpp
boost::asio::async_write(*socket_,
    boost::asio::buffer(data),
    boost::asio::bind_executor(strand_,
        [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (error && error != boost::asio::error::operation_aborted) {
                connected_ = false;
                notifyConnectionStatus(false, error.message());
            }
        }
    )
);
```

### 3. 接收错误

接收错误通过异步读取操作的回调函数处理：

```cpp
void AsioNetworkModel::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        // 处理接收到的数据
    } else if (error != boost::asio::error::operation_aborted) {
        // 处理错误
        connected_ = false;
        notifyConnectionStatus(false, error.message());
    }
}
```

### 4. 解析错误

消息解析错误在 `X30Protocol::parseReceivedData` 方法中处理：

```cpp
std::vector<std::shared_ptr<IMessage>> X30Protocol::parseReceivedData(const std::vector<char>& data, size_t bytes_transferred) {
    std::vector<std::shared_ptr<IMessage>> messages;

    try {
        // 解析数据
        std::string data_str(data.begin(), data.begin() + bytes_transferred);

        // 提取消息类型
        MessageType type = extractMessageType(data_str);

        // 处理 XML 数据
        auto message = handleXmlData(data_str, type);
        if (message) {
            messages.push_back(message);
        }
    } catch (const std::exception& e) {
        // 记录错误，但不中断处理
    }

    return messages;
}
```

## 自动重连机制

SDK 实现了自动重连机制，当连接断开时自动尝试重新连接：

```cpp
void NavigationSdkImpl::handleConnectionStatus(bool connected, const std::string& error_message) {
    if (connected_ && !connected) {
        // 连接断开
        connected_ = false;

        // 触发断开连接事件
        if (event_callback_) {
            Event event;
            event.type = EventType::DISCONNECTED;
            event.message = error_message;
            event_callback_(event);
        }

        // 如果启用了自动重连，则尝试重新连接
        if (auto_reconnect_) {
            startReconnectTimer();
        }
    } else if (!connected_ && connected) {
        // 连接建立
        connected_ = true;

        // 触发连接建立事件
        if (event_callback_) {
            Event event;
            event.type = EventType::CONNECTION_ESTABLISHED;
            event_callback_(event);
        }

        // 取消重连定时器
        stopReconnectTimer();
    }
}

void NavigationSdkImpl::startReconnectTimer() {
    reconnect_timer_ = std::make_unique<boost::asio::steady_timer>(io_context_);
    reconnect_timer_->expires_after(std::chrono::seconds(reconnect_interval_));
    reconnect_timer_->async_wait([this](const boost::system::error_code& error) {
        if (!error) {
            // 尝试重新连接
            if (!connected_) {
                network_model_->connect(host_, port_);
            }
        }
    });
}

void NavigationSdkImpl::stopReconnectTimer() {
    if (reconnect_timer_) {
        reconnect_timer_->cancel();
        reconnect_timer_.reset();
    }
}
```

## 超时处理

SDK 实现了请求超时处理机制，确保请求不会无限期等待：

```cpp
template<typename ResponseType>
std::pair<std::shared_ptr<ResponseType>, ErrorCode> NavigationSdkImpl::sendRequestAndWaitResponse(
    const std::shared_ptr<IMessage>& request,
    MessageType expected_response_type,
    const std::chrono::milliseconds& timeout) {

    if (!connected_) {
        return {nullptr, ErrorCode::NOT_CONNECTED};
    }

    std::mutex mutex;
    std::condition_variable cv;
    std::shared_ptr<ResponseType> response;
    bool response_received = false;

    // 生成请求 ID
    uint32_t request_id = generateRequestId();
    request->setRequestId(request_id);

    // 注册待处理请求
    {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_[request_id] = [&](const std::shared_ptr<IMessage>& msg) {
            if (msg->getType() == expected_response_type) {
                std::lock_guard<std::mutex> lock(mutex);
                response = std::dynamic_pointer_cast<ResponseType>(msg);
                response_received = true;
                cv.notify_one();
            }
        };
    }

    // 发送请求
    if (!network_model_->sendMessage(request)) {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_.erase(request_id);
        return {nullptr, ErrorCode::SEND_FAILED};
    }

    // 等待响应或超时
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (!cv.wait_for(lock, timeout, [&]() { return response_received; })) {
            // 超时
            std::lock_guard<std::mutex> request_lock(pending_requests_mutex_);
            pending_requests_.erase(request_id);
            return {nullptr, ErrorCode::TIMEOUT};
        }
    }

    // 移除待处理请求
    {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_.erase(request_id);
    }

    return {response, ErrorCode::SUCCESS};
}
```

## 最佳实践

在使用 SDK 的网络通信功能时，建议遵循以下最佳实践：

### 1. 连接管理

- 在应用程序启动时建立连接，在退出前断开连接
- 启用自动重连功能，确保连接的可靠性
- 监听连接状态变化事件，及时处理连接断开情况

```cpp
// 启用自动重连
sdk.enableAutoReconnect(true);

// 设置事件回调
sdk.setEventCallback([](const Event& event) {
    if (event.type == EventType::CONNECTION_ESTABLISHED) {
        std::cout << "连接已建立" << std::endl;
    } else if (event.type == EventType::DISCONNECTED) {
        std::cout << "连接已断开: " << event.message << std::endl;
    }
});

// 建立连接
sdk.connect("192.168.1.100", 8080);

// 应用程序退出前断开连接
sdk.disconnect();
```

### 2. 异步操作

- 使用异步方法避免阻塞主线程
- 正确处理异步操作的回调函数
- 避免在回调函数中执行长时间操作

```cpp
// 使用异步方法获取实时状态
sdk.getRealTimeStatusAsync([](const RealTimeStatus& status, ErrorCode error) {
    if (error == ErrorCode::SUCCESS) {
        // 处理实时状态
    } else {
        // 处理错误
    }
});

// 使用异步方法开始导航任务
sdk.startNavigationAsync(navigation_points, [](const NavigationResult& result, ErrorCode error) {
    if (error == ErrorCode::SUCCESS) {
        // 处理导航结果
    } else {
        // 处理错误
    }
});
```

### 3. 错误处理

- 检查所有操作的返回值和错误码
- 实现适当的错误恢复机制
- 记录错误信息，便于调试和问题排查

```cpp
// 同步方法错误处理
auto [status, error] = sdk.getRealTimeStatus();
if (error != ErrorCode::SUCCESS) {
    std::cerr << "获取实时状态失败: " << getErrorMessage(error) << std::endl;
    // 实现错误恢复机制
}

// 异步方法错误处理
sdk.startNavigationAsync(navigation_points, [](const NavigationResult& result, ErrorCode error) {
    if (error != ErrorCode::SUCCESS) {
        std::cerr << "开始导航任务失败: " << getErrorMessage(error) << std::endl;
        // 实现错误恢复机制
    }
});
```

## 总结

X30 机器狗导航 SDK 的网络通信模型基于 TCP/IP 协议和 Boost.Asio 库，提供了可靠、高效的异步通信机制。SDK 支持 XML 和 JSON 消息格式，实现了完善的错误处理和自动重连机制，确保通信的稳定性和可靠性。

开发者在使用 SDK 时，应遵循最佳实践，正确管理连接、使用异步操作和处理错误，以充分发挥 SDK 的性能和功能。
