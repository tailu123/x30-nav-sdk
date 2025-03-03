# 错误处理

X30 机器狗导航 SDK 提供了全面的错误处理机制，帮助开发者识别、诊断和处理各种错误情况。本文档详细介绍 SDK 的错误处理机制、错误码定义以及最佳实践。

## 错误处理机制

SDK 使用多种机制处理错误：

1. **错误码（ErrorCode）**：每个操作都返回错误码，表示操作的结果
2. **异常处理**：内部使用异常处理机制，但不向外部抛出异常
3. **错误消息**：提供详细的错误消息，帮助诊断问题
4. **事件通知**：通过事件回调通知错误事件
5. **日志记录**：记录错误信息，便于调试和问题排查

## 错误码定义

SDK 定义了以下错误码：

```cpp
enum class ErrorCode {
    SUCCESS = 0,            // 成功
    NOT_CONNECTED,          // 未连接
    TIMEOUT,                // 超时
    INVALID_PARAMETER,      // 无效参数
    SEND_FAILED,            // 发送失败
    RECEIVE_FAILED,         // 接收失败
    PARSE_FAILED,           // 解析失败
    TASK_NOT_FOUND,         // 任务未找到
    SERVER_ERROR,           // 服务器错误
    UNKNOWN_ERROR           // 未知错误
};
```

### 错误码详解

| 错误码 | 描述 | 可能的原因 | 建议的处理方法 |
|--------|------|------------|--------------|
| `SUCCESS` | 操作成功 | 操作正常完成 | 继续执行后续操作 |
| `NOT_CONNECTED` | 未连接到机器狗控制系统 | 网络连接断开、连接超时、服务器未启动 | 检查网络连接，尝试重新连接 |
| `TIMEOUT` | 操作超时 | 网络延迟、服务器响应慢、请求丢失 | 增加超时时间，检查网络状况，重试操作 |
| `INVALID_PARAMETER` | 参数无效 | 参数格式错误、参数值超出范围、必需参数缺失 | 检查参数值，确保符合要求 |
| `SEND_FAILED` | 发送消息失败 | 网络连接断开、缓冲区已满、套接字错误 | 检查网络连接，尝试重新连接 |
| `RECEIVE_FAILED` | 接收消息失败 | 网络连接断开、数据损坏、套接字错误 | 检查网络连接，尝试重新连接 |
| `PARSE_FAILED` | 解析消息失败 | 消息格式错误、协议版本不匹配、数据损坏 | 检查消息格式，确保符合协议要求 |
| `TASK_NOT_FOUND` | 任务未找到 | 任务 ID 无效、任务已完成或取消、服务器重启 | 检查任务 ID，查询任务状态 |
| `SERVER_ERROR` | 服务器错误 | 服务器内部错误、资源不足、配置错误 | 联系服务器管理员，检查服务器日志 |
| `UNKNOWN_ERROR` | 未知错误 | 未预期的错误、未分类的错误 | 检查日志，联系技术支持 |

## 同步方法的错误处理

同步方法返回一个包含结果和错误码的对（`std::pair`）：

```cpp
// 获取实时状态
auto [status, error] = sdk.getRealTimeStatus();
if (error == ErrorCode::SUCCESS) {
    // 处理实时状态
    std::cout << "纬度: " << status.latitude << ", 经度: " << status.longitude << std::endl;
} else {
    // 处理错误
    std::cerr << "获取实时状态失败: " << getErrorMessage(error) << std::endl;

    // 根据错误码采取不同的处理方法
    switch (error) {
        case ErrorCode::NOT_CONNECTED:
            // 尝试重新连接
            sdk.connect(host, port);
            break;
        case ErrorCode::TIMEOUT:
            // 增加超时时间，重试
            std::tie(status, error) = sdk.getRealTimeStatus(std::chrono::seconds(10));
            break;
        default:
            // 其他错误处理
            break;
    }
}
```

## 异步方法的错误处理

异步方法在回调函数中提供结果和错误码：

```cpp
// 获取实时状态
sdk.getRealTimeStatusAsync([](const RealTimeStatus& status, ErrorCode error) {
    if (error == ErrorCode::SUCCESS) {
        // 处理实时状态
        std::cout << "纬度: " << status.latitude << ", 经度: " << status.longitude << std::endl;
    } else {
        // 处理错误
        std::cerr << "获取实时状态失败: " << getErrorMessage(error) << std::endl;

        // 根据错误码采取不同的处理方法
        switch (error) {
            case ErrorCode::NOT_CONNECTED:
                // 处理未连接错误
                break;
            case ErrorCode::TIMEOUT:
                // 处理超时错误
                break;
            default:
                // 处理其他错误
                break;
        }
    }
});
```

## 事件回调中的错误处理

SDK 通过事件回调通知错误事件：

```cpp
// 设置事件回调
sdk.setEventCallback([](const Event& event) {
    if (event.type == EventType::ERROR) {
        // 处理错误事件
        std::cerr << "错误事件: " << event.message << std::endl;

        // 根据错误消息采取不同的处理方法
        if (event.message.find("connection") != std::string::npos) {
            // 处理连接相关错误
        } else if (event.message.find("timeout") != std::string::npos) {
            // 处理超时相关错误
        } else {
            // 处理其他错误
        }
    } else if (event.type == EventType::DISCONNECTED) {
        // 处理断开连接事件
        std::cerr << "断开连接: " << event.message << std::endl;

        // 尝试重新连接
    }
});
```

## 获取错误消息

SDK 提供 `getErrorMessage` 函数，将错误码转换为可读的错误消息：

```cpp
/**
 * @brief 获取错误码对应的错误消息
 * @param error 错误码
 * @return 错误消息
 */
std::string getErrorMessage(ErrorCode error) {
    switch (error) {
        case ErrorCode::SUCCESS:
            return "成功";
        case ErrorCode::NOT_CONNECTED:
            return "未连接到机器狗控制系统";
        case ErrorCode::TIMEOUT:
            return "操作超时";
        case ErrorCode::INVALID_PARAMETER:
            return "参数无效";
        case ErrorCode::SEND_FAILED:
            return "发送消息失败";
        case ErrorCode::RECEIVE_FAILED:
            return "接收消息失败";
        case ErrorCode::PARSE_FAILED:
            return "解析消息失败";
        case ErrorCode::TASK_NOT_FOUND:
            return "任务未找到";
        case ErrorCode::SERVER_ERROR:
            return "服务器错误";
        case ErrorCode::UNKNOWN_ERROR:
            return "未知错误";
        default:
            return "未定义的错误";
    }
}
```

## 常见错误场景及处理方法

### 1. 连接错误

```cpp
// 连接到机器狗控制系统
if (!sdk.connect("192.168.1.100", 8080)) {
    std::cerr << "连接请求发送失败" << std::endl;
    // 检查网络连接
    // 检查主机地址和端口
    // 尝试重新连接
}

// 设置事件回调监听连接状态
sdk.setEventCallback([&](const Event& event) {
    if (event.type == EventType::DISCONNECTED) {
        std::cerr << "断开连接: " << event.message << std::endl;

        // 启用自动重连
        sdk.enableAutoReconnect(true);

        // 或者手动重连
        std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            sdk.connect("192.168.1.100", 8080);
        }).detach();
    }
});
```

### 2. 超时错误

```cpp
// 增加超时时间
auto [status, error] = sdk.getRealTimeStatus(std::chrono::seconds(10));
if (error == ErrorCode::TIMEOUT) {
    std::cerr << "获取实时状态超时" << std::endl;

    // 重试操作
    std::tie(status, error) = sdk.getRealTimeStatus(std::chrono::seconds(15));

    // 或者使用异步方法避免阻塞
    sdk.getRealTimeStatusAsync([](const RealTimeStatus& status, ErrorCode error) {
        // 处理结果
    }, std::chrono::seconds(15));
}
```

### 3. 任务未找到错误

```cpp
// 查询任务状态
auto [task_status, error] = sdk.queryTaskStatus(task_id);
if (error == ErrorCode::TASK_NOT_FOUND) {
    std::cerr << "任务未找到: " << task_id << std::endl;

    // 获取当前活动的任务列表
    auto [real_time_status, rt_error] = sdk.getRealTimeStatus();
    if (rt_error == ErrorCode::SUCCESS) {
        std::cout << "当前活动的任务: ";
        for (const auto& id : real_time_status.task_ids) {
            std::cout << id << " ";
        }
        std::cout << std::endl;
    }
}
```

### 4. 解析错误

```cpp
// 处理解析错误
if (error == ErrorCode::PARSE_FAILED) {
    std::cerr << "解析消息失败" << std::endl;

    // 检查协议版本
    std::cout << "SDK 版本: " << sdk.getVersion() << std::endl;

    // 重试操作
    std::tie(status, error) = sdk.getRealTimeStatus();
}
```

## 错误恢复策略

### 1. 重试机制

对于可恢复的错误，可以实现重试机制：

```cpp
// 重试函数模板
template<typename Func, typename... Args>
auto retryOperation(Func func, int max_retries, std::chrono::milliseconds delay, Args&&... args) {
    using ReturnType = decltype(func(std::forward<Args>(args)...));

    ReturnType result;
    int retries = 0;

    do {
        result = func(std::forward<Args>(args)...);

        // 检查是否成功
        if (std::get<1>(result) == ErrorCode::SUCCESS) {
            return result;
        }

        // 增加重试次数
        retries++;

        // 如果还有重试次数，等待一段时间
        if (retries < max_retries) {
            std::this_thread::sleep_for(delay);

            // 指数退避
            delay *= 2;
        }
    } while (retries < max_retries);

    return result;
}

// 使用重试机制
auto [status, error] = retryOperation(
    [&](auto timeout) { return sdk.getRealTimeStatus(timeout); },
    3,  // 最大重试次数
    std::chrono::milliseconds(500),  // 初始延迟
    std::chrono::seconds(5)  // 超时参数
);
```

### 2. 断路器模式

对于可能导致资源浪费的错误，可以实现断路器模式：

```cpp
class CircuitBreaker {
public:
    CircuitBreaker(int failure_threshold, std::chrono::milliseconds reset_timeout)
        : failure_threshold_(failure_threshold), reset_timeout_(reset_timeout),
          state_(State::CLOSED), failures_(0), last_failure_time_() {}

    template<typename Func, typename... Args>
    auto execute(Func func, Args&&... args) {
        using ReturnType = decltype(func(std::forward<Args>(args)...));

        // 检查断路器状态
        if (state_ == State::OPEN) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_failure_time_);

            if (elapsed >= reset_timeout_) {
                // 进入半开状态
                state_ = State::HALF_OPEN;
            } else {
                // 断路器打开，返回错误
                return ReturnType{std::get<0>(ReturnType{}), ErrorCode::CIRCUIT_OPEN};
            }
        }

        // 执行操作
        auto result = func(std::forward<Args>(args)...);

        // 检查结果
        if (std::get<1>(result) != ErrorCode::SUCCESS) {
            // 操作失败
            failures_++;
            last_failure_time_ = std::chrono::steady_clock::now();

            if (state_ == State::HALF_OPEN || failures_ >= failure_threshold_) {
                // 进入打开状态
                state_ = State::OPEN;
            }
        } else {
            // 操作成功
            if (state_ == State::HALF_OPEN) {
                // 恢复到关闭状态
                state_ = State::CLOSED;
            }

            // 重置失败计数
            failures_ = 0;
        }

        return result;
    }

private:
    enum class State {
        CLOSED,     // 正常状态
        OPEN,       // 断路器打开状态
        HALF_OPEN   // 半开状态
    };

    int failure_threshold_;
    std::chrono::milliseconds reset_timeout_;
    State state_;
    int failures_;
    std::chrono::steady_clock::time_point last_failure_time_;
};

// 使用断路器
CircuitBreaker circuit_breaker(3, std::chrono::seconds(30));

auto [status, error] = circuit_breaker.execute(
    [&](auto timeout) { return sdk.getRealTimeStatus(timeout); },
    std::chrono::seconds(5)
);
```

## 日志记录

SDK 内部实现了日志记录机制，记录错误信息和操作日志。开发者可以通过设置日志级别和日志回调函数，自定义日志处理：

```cpp
// 设置日志级别
sdk.setLogLevel(LogLevel::DEBUG);

// 设置日志回调函数
sdk.setLogCallback([](LogLevel level, const std::string& message) {
    std::string level_str;
    switch (level) {
        case LogLevel::ERROR:
            level_str = "ERROR";
            break;
        case LogLevel::WARNING:
            level_str = "WARNING";
            break;
        case LogLevel::INFO:
            level_str = "INFO";
            break;
        case LogLevel::DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    std::cout << "[" << level_str << "] " << message << std::endl;

    // 记录到文件
    std::ofstream log_file("sdk.log", std::ios::app);
    if (log_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        log_file << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " ["
                 << level_str << "] " << message << std::endl;
    }
});
```

## 最佳实践

### 1. 始终检查错误码

```cpp
// 同步方法
auto [result, error] = sdk.someOperation();
if (error != ErrorCode::SUCCESS) {
    // 处理错误
}

// 异步方法
sdk.someOperationAsync([](const Result& result, ErrorCode error) {
    if (error != ErrorCode::SUCCESS) {
        // 处理错误
    }
});
```

### 2. 实现适当的错误恢复机制

```cpp
// 连接错误恢复
sdk.setEventCallback([&](const Event& event) {
    if (event.type == EventType::DISCONNECTED) {
        // 启用自动重连
        sdk.enableAutoReconnect(true);
    }
});

// 超时错误恢复
if (error == ErrorCode::TIMEOUT) {
    // 增加超时时间，重试
    std::tie(result, error) = sdk.someOperation(std::chrono::seconds(10));
}
```

### 3. 使用异步方法避免阻塞

```cpp
// 使用异步方法
sdk.someOperationAsync([](const Result& result, ErrorCode error) {
    // 处理结果和错误
}, std::chrono::seconds(10));
```

### 4. 记录错误信息

```cpp
// 记录错误信息
if (error != ErrorCode::SUCCESS) {
    std::cerr << "操作失败: " << getErrorMessage(error) << std::endl;

    // 记录到日志文件
    logError("操作失败: " + getErrorMessage(error));
}
```

### 5. 优雅处理连接断开

```cpp
// 设置事件回调
sdk.setEventCallback([&](const Event& event) {
    if (event.type == EventType::DISCONNECTED) {
        std::cerr << "断开连接: " << event.message << std::endl;

        // 通知用户
        notifyUser("与机器狗控制系统的连接已断开");

        // 启用自动重连
        sdk.enableAutoReconnect(true);
    }
});
```

## 总结

X30 机器狗导航 SDK 提供了全面的错误处理机制，包括错误码、错误消息、事件通知和日志记录。开发者应该始终检查错误码，实现适当的错误恢复机制，使用异步方法避免阻塞，记录错误信息，并优雅处理连接断开等错误情况。

通过遵循本文档中的最佳实践，开发者可以构建更加健壮和可靠的应用程序，提供更好的用户体验。
