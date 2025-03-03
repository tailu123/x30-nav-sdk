# 错误处理优化

## 当前错误处理机制分析

X30 机器狗导航 SDK 目前的错误处理机制主要包括以下几个方面：

1. **错误码（ErrorCode）**：通过枚举类型定义各种错误状态
2. **异常处理**：在关键操作中使用 try-catch 捕获异常
3. **回调安全包装**：使用 `safeCallback` 函数包装用户回调，防止异常传播
4. **超时处理**：对网络请求设置超时机制
5. **日志记录**：记录错误和异常信息

### 现有问题

1. **错误处理不统一**：不同模块使用不同的错误处理方式
2. **错误信息不详细**：错误码缺乏详细的错误描述和上下文信息
3. **错误恢复机制不完善**：缺乏自动恢复和重试机制
4. **并发安全性问题**：在高并发场景下可能出现死锁、数据竞争和网络数据混乱

## 并发安全性分析

### 死锁风险分析

当前 SDK 中存在以下潜在死锁风险：

1. **锁的嵌套使用**：在 `NavigationSdkImpl` 类中，有多个互斥锁（`network_mutex_`、`pending_requests_mutex_`、`navigation_result_callbacks_mutex_`），如果嵌套使用可能导致死锁。

2. **回调中的锁操作**：如果用户回调函数中再次调用 SDK 方法，可能导致锁的循环等待。

3. **资源获取顺序不一致**：不同方法中获取多个锁的顺序不一致，可能导致死锁。

### 网络数据混乱风险分析

1. **消息序列号管理**：当前使用 `sequenceNumber` 标识请求和响应的对应关系，但在高并发场景下可能出现序列号重复或管理不当的问题。

2. **并发请求处理**：多个线程同时发送请求时，可能导致请求和响应的匹配错误。

3. **缓冲区管理**：接收缓冲区的管理在并发场景下可能不安全。

### 数据竞争风险分析

1. **共享状态访问**：多个线程访问共享状态（如连接状态、请求队列）时可能发生数据竞争。

2. **回调执行环境**：回调函数在 IO 线程中执行，可能与用户线程产生数据竞争。

3. **资源释放时机**：在断开连接或销毁 SDK 实例时，可能存在资源释放与访问的竞争条件。

## 优化建议

### 1. 统一错误处理框架

创建一个统一的错误处理框架，包括：

```cpp
// 增强的错误信息结构
struct ErrorInfo {
    ErrorCode code;                 // 错误码
    std::string message;            // 错误消息
    std::string context;            // 错误上下文
    std::chrono::system_clock::time_point timestamp; // 错误发生时间

    // 可选的调用堆栈信息
    std::vector<std::string> callStack;

    std::string toString() const;
};

// 错误处理结果模板
template<typename T>
class Result {
public:
    // 成功构造函数
    static Result<T> success(const T& value) {
        return Result<T>(value);
    }

    // 失败构造函数
    static Result<T> failure(const ErrorInfo& error) {
        return Result<T>(error);
    }

    // 检查是否成功
    bool isSuccess() const { return !hasError_; }

    // 获取值（如果失败则抛出异常）
    const T& value() const;

    // 获取错误信息（如果成功则返回默认错误）
    const ErrorInfo& error() const;

private:
    bool hasError_;
    T value_;
    ErrorInfo error_;

    Result(const T& value) : hasError_(false), value_(value) {}
    Result(const ErrorInfo& error) : hasError_(true), error_(error) {}
};
```

### 2. 改进锁策略，防止死锁

1. **定义锁的获取顺序**：确保所有方法按照相同的顺序获取锁。

```cpp
// 示例：按照固定顺序获取锁
void someMethod() {
    // 始终按照这个顺序获取锁
    std::lock_guard<std::mutex> lock1(network_mutex_);
    std::lock_guard<std::mutex> lock2(pending_requests_mutex_);
    std::lock_guard<std::mutex> lock3(navigation_result_callbacks_mutex_);

    // 业务逻辑
}
```

2. **使用 `std::lock` 同时锁定多个互斥量**：

```cpp
void someMethod() {
    std::unique_lock<std::mutex> lock1(network_mutex_, std::defer_lock);
    std::unique_lock<std::mutex> lock2(pending_requests_mutex_, std::defer_lock);
    std::lock(lock1, lock2); // 原子地锁定多个互斥量，避免死锁

    // 业务逻辑
}
```

3. **减少锁的粒度**：缩小临界区范围，减少持有锁的时间。

4. **避免在持有锁的情况下调用用户回调**：

```cpp
// 错误的方式
void onMessageReceived(std::unique_ptr<protocol::IMessage> message) {
    std::lock_guard<std::mutex> lock(mutex_);
    // 处理消息
    callback(result); // 在持有锁的情况下调用用户回调
}

// 正确的方式
void onMessageReceived(std::unique_ptr<protocol::IMessage> message) {
    NavigationResultCallback callbackCopy;
    NavigationResult resultCopy;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        // 处理消息
        callbackCopy = callback;
        resultCopy = result;
    }

    // 在释放锁后调用用户回调
    if (callbackCopy) {
        callbackCopy(resultCopy);
    }
}
```

### 3. 增强网络数据处理安全性

1. **改进序列号生成机制**：使用线程安全的序列号生成器，确保序列号唯一。

```cpp
class SequenceNumberGenerator {
public:
    uint16_t generate() {
        return counter_.fetch_add(1, std::memory_order_relaxed) % UINT16_MAX;
    }

private:
    std::atomic<uint16_t> counter_{0};
};
```

2. **增强消息匹配机制**：使用更可靠的方式匹配请求和响应。

```cpp
struct PendingRequest {
    protocol::MessageType expectedResponseType;
    std::string requestId; // 增加唯一请求ID
    std::unique_ptr<protocol::IMessage> response;
    bool responseReceived;
    std::shared_ptr<std::condition_variable> cv;
    std::chrono::system_clock::time_point expirationTime; // 增加过期时间
};
```

3. **实现请求超时清理机制**：定期清理过期的请求。

```cpp
void cleanupExpiredRequests() {
    std::lock_guard<std::mutex> lock(pending_requests_mutex_);
    auto now = std::chrono::system_clock::now();

    for (auto it = pendingRequests_.begin(); it != pendingRequests_.end();) {
        if (now > it->second.expirationTime) {
            it->second.cv->notify_all(); // 通知等待线程
            it = pendingRequests_.erase(it);
        } else {
            ++it;
        }
    }
}
```

### 4. 防止数据竞争

1. **使用原子操作**：对于简单的状态标志，使用原子变量而不是互斥锁。

```cpp
std::atomic<bool> connected_{false};
std::atomic<int> pendingRequestCount_{0};
```

2. **使用线程安全的数据结构**：考虑使用无锁数据结构或线程安全容器。

3. **实现线程安全的回调执行机制**：

```cpp
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    if (!callback) return;

    try {
        // 在 strand 中执行回调，确保线程安全
        boost::asio::post(strand_, [callback, args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            std::apply(callback, std::move(args));
        });
    } catch (const std::exception& e) {
        // 记录异常
        std::cerr << "Exception in " << callbackType << " callback: " << e.what() << std::endl;
    } catch (...) {
        // 记录未知异常
        std::cerr << "Unknown exception in " << callbackType << " callback" << std::endl;
    }
}
```

### 5. 增强资源管理

1. **使用 RAII 原则**：确保资源在获取和释放时遵循 RAII 原则。

2. **实现安全的关闭流程**：

```cpp
void disconnect() {
    // 标记断开连接状态
    bool expected = true;
    if (!connected_.compare_exchange_strong(expected, false)) {
        return; // 已经断开连接
    }

    // 取消所有挂起的请求
    {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        for (auto& request : pendingRequests_) {
            request.second.cv->notify_all();
        }
    }

    // 断开网络连接
    network_model_->disconnect();

    // 触发断开连接事件
    Event event;
    event.type = EventType::DISCONNECTED;
    event.timestamp = std::chrono::system_clock::now();
    event.message = "主动断开连接";

    if (event_callback_) {
        safeCallback(event_callback_, "事件", event);
    }
}
```

3. **实现优雅的资源释放**：

```cpp
~NavigationSdkImpl() {
    // 断开连接
    disconnect();

    // 等待所有回调完成
    // 这里可以使用一个计数器或其他机制确保所有回调都已完成

    // 清理资源
    navigation_result_callbacks_.clear();
    pendingRequests_.clear();
}
```

## 实施计划

1. **短期优化**：
   - 修复现有的死锁风险
   - 改进锁的使用策略
   - 增强序列号生成机制

2. **中期优化**：
   - 实现统一的错误处理框架
   - 增强网络数据处理安全性
   - 改进资源管理

3. **长期优化**：
   - 重构为无锁设计（在可能的情况下）
   - 实现更高效的并发模型
   - 增加全面的并发测试

## 总结

通过实施上述优化建议，X30 机器狗导航 SDK 可以显著提高在高并发场景下的安全性和可靠性，有效防止死锁、网络数据混乱和数据竞争问题。这些优化不仅能提高 SDK 的稳定性，还能提升性能和用户体验。

在实施过程中，应保持向后兼容性，避免破坏现有的 API 和功能。同时，应该增加全面的并发测试，确保优化措施的有效性。
