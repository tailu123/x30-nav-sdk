# 日志系统

X30 机器狗导航 SDK 的日志系统是监控、调试和排查问题的重要工具。本文档详细介绍当前日志系统的实现、存在的问题以及优化建议。

## 当前日志系统现状

目前，SDK 的日志系统主要依赖于标准输出和标准错误流，缺乏完整的日志框架。日志记录主要出现在以下几个方面：

1. **异常处理**：在捕获异常时输出错误信息
2. **网络通信**：记录网络连接、断开和错误
3. **消息解析**：记录消息解析过程中的错误
4. **回调函数异常**：记录用户回调函数中抛出的异常

### 现有日志实现示例

```cpp
// 回调函数异常处理
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    try {
        callback(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数异常: " << e.what() << std::endl;
    } catch (...) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

        std::cerr << "[" << ss.str() << "] " << callbackType << " 回调函数发生未知异常" << std::endl;
    }
}

// 网络错误处理
void AsioNetworkModel::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        if (error != boost::asio::error::operation_aborted) {
            std::cerr << "接收数据错误: " << error.message() << std::endl;
            disconnect();
        }
        return;
    }
    // ...
}

// 消息解析错误处理
std::unique_ptr<IMessage> X30Protocol::parseReceivedData(const std::string& data) {
    try {
        // ...
    } catch (const std::exception& e) {
        std::cerr << "解析数据异常: " << e.what() << std::endl;
        return nullptr;
    }
}
```

### 现有日志系统的问题

1. **缺乏统一接口**：日志记录分散在各个模块，没有统一的接口
2. **日志级别控制不足**：无法根据需要调整日志级别
3. **日志输出单一**：只能输出到标准错误流，不支持文件、网络等其他输出目标
4. **日志格式不一致**：不同模块的日志格式不统一，难以解析和分析
5. **缺乏上下文信息**：日志中缺少模块、函数、行号等上下文信息
6. **线程安全性问题**：多线程环境下可能出现日志混乱
7. **性能考虑不足**：日志记录可能影响性能，尤其是在高频操作中

## 日志系统优化建议

为了解决上述问题，建议实现一个完整的日志系统，具有以下特性：

### 1. 统一的日志接口

```cpp
namespace logging {
    // 日志级别
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL,
        OFF
    };

    // 日志记录函数
    void log(Level level, const std::string& message);
    void log(Level level, const char* format, ...);

    // 便捷宏
    #define LOG_TRACE(format, ...) logging::log(logging::Level::TRACE, format, ##__VA_ARGS__)
    #define LOG_DEBUG(format, ...) logging::log(logging::Level::DEBUG, format, ##__VA_ARGS__)
    #define LOG_INFO(format, ...)  logging::log(logging::Level::INFO, format, ##__VA_ARGS__)
    #define LOG_WARN(format, ...)  logging::log(logging::Level::WARNING, format, ##__VA_ARGS__)
    #define LOG_ERROR(format, ...) logging::log(logging::Level::ERROR, format, ##__VA_ARGS__)
    #define LOG_FATAL(format, ...) logging::log(logging::Level::FATAL, format, ##__VA_ARGS__)
}
```

### 2. 可配置的日志级别

```cpp
namespace logging {
    // 设置全局日志级别
    void setLevel(Level level);

    // 获取当前日志级别
    Level getLevel();

    // 检查日志级别是否启用
    bool isLevelEnabled(Level level);
}
```

### 3. 多目标日志输出

```cpp
namespace logging {
    // 日志目标接口
    class LogTarget {
    public:
        virtual ~LogTarget() = default;
        virtual void write(Level level, const std::string& message) = 0;
        virtual void flush() = 0;
    };

    // 控制台日志目标
    class ConsoleTarget : public LogTarget {
    public:
        void write(Level level, const std::string& message) override;
        void flush() override;
    };

    // 文件日志目标
    class FileTarget : public LogTarget {
    public:
        explicit FileTarget(const std::string& filename);
        void write(Level level, const std::string& message) override;
        void flush() override;
    private:
        std::ofstream file_;
    };

    // 添加日志目标
    void addTarget(std::shared_ptr<LogTarget> target);

    // 移除日志目标
    void removeTarget(std::shared_ptr<LogTarget> target);

    // 移除所有日志目标
    void clearTargets();
}
```

### 4. 统一的日志格式

```cpp
namespace logging {
    // 日志格式化器接口
    class Formatter {
    public:
        virtual ~Formatter() = default;
        virtual std::string format(Level level, const std::string& message) = 0;
    };

    // 默认格式化器
    class DefaultFormatter : public Formatter {
    public:
        std::string format(Level level, const std::string& message) override;
    };

    // 设置格式化器
    void setFormatter(std::shared_ptr<Formatter> formatter);
}
```

### 5. 丰富的上下文信息

```cpp
namespace logging {
    // 日志事件结构
    struct LogEvent {
        Level level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::thread::id threadId;
        std::string fileName;
        std::string functionName;
        int lineNumber;
    };

    // 带上下文信息的日志宏
    #define LOG_DEBUG_CTX(format, ...) \
        logging::logWithContext(logging::Level::DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

    // 带上下文信息的日志函数
    void logWithContext(Level level, const char* file, const char* function, int line, const char* format, ...);
}
```

### 6. 线程安全的实现

```cpp
namespace logging {
    namespace detail {
        // 线程安全的日志队列
        class LogQueue {
        public:
            void push(const LogEvent& event);
            bool pop(LogEvent& event);

        private:
            std::mutex mutex_;
            std::queue<LogEvent> queue_;
            std::condition_variable cv_;
        };

        // 日志线程
        class LogThread {
        public:
            LogThread();
            ~LogThread();

            void start();
            void stop();

        private:
            void threadFunc();

            std::thread thread_;
            std::atomic<bool> running_;
        };
    }
}
```

### 7. 性能优化

```cpp
namespace logging {
    // 异步日志
    void enableAsync(bool enable);

    // 批量写入
    void setBatchSize(size_t size);

    // 日志缓冲区
    class LogBuffer {
    public:
        explicit LogBuffer(size_t size);
        void append(const char* data, size_t len);
        void flush();

    private:
        std::vector<char> buffer_;
        size_t used_;
    };
}
```

## 实现示例

以下是一个简化的日志系统实现示例：

```cpp
// logging.hpp
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <atomic>

namespace logging {

enum class Level {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    OFF
};

class LogTarget {
public:
    virtual ~LogTarget() = default;
    virtual void write(Level level, const std::string& message) = 0;
    virtual void flush() = 0;
};

class ConsoleTarget : public LogTarget {
public:
    void write(Level level, const std::string& message) override {
        if (level >= Level::ERROR) {
            std::cerr << message << std::endl;
        } else {
            std::cout << message << std::endl;
        }
    }

    void flush() override {
        std::cout.flush();
        std::cerr.flush();
    }
};

class FileTarget : public LogTarget {
public:
    explicit FileTarget(const std::string& filename) {
        file_.open(filename, std::ios::app);
    }

    ~FileTarget() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void write(Level level, const std::string& message) override {
        if (file_.is_open()) {
            file_ << message << std::endl;
        }
    }

    void flush() override {
        if (file_.is_open()) {
            file_.flush();
        }
    }

private:
    std::ofstream file_;
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLevel(Level level) {
        level_ = level;
    }

    Level getLevel() const {
        return level_;
    }

    bool isLevelEnabled(Level level) const {
        return level >= level_;
    }

    void addTarget(std::shared_ptr<LogTarget> target) {
        std::lock_guard<std::mutex> lock(mutex_);
        targets_.push_back(target);
    }

    void removeTarget(std::shared_ptr<LogTarget> target) {
        std::lock_guard<std::mutex> lock(mutex_);
        targets_.erase(
            std::remove(targets_.begin(), targets_.end(), target),
            targets_.end()
        );
    }

    void clearTargets() {
        std::lock_guard<std::mutex> lock(mutex_);
        targets_.clear();
    }

    void log(Level level, const std::string& message) {
        if (!isLevelEnabled(level)) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << "] ";
        ss << "[" << levelToString(level) << "] ";
        ss << "[" << std::this_thread::get_id() << "] ";
        ss << message;

        std::string formatted = ss.str();

        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& target : targets_) {
            target->write(level, formatted);
        }
    }

    void flush() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& target : targets_) {
            target->flush();
        }
    }

private:
    Logger() : level_(Level::INFO) {
        // 默认添加控制台日志目标
        addTarget(std::make_shared<ConsoleTarget>());
    }

    ~Logger() {
        flush();
    }

    std::string levelToString(Level level) const {
        switch (level) {
            case Level::TRACE: return "TRACE";
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            case Level::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    Level level_;
    std::vector<std::shared_ptr<LogTarget>> targets_;
    std::mutex mutex_;
};

// 全局函数
inline void setLevel(Level level) {
    Logger::getInstance().setLevel(level);
}

inline Level getLevel() {
    return Logger::getInstance().getLevel();
}

inline bool isLevelEnabled(Level level) {
    return Logger::getInstance().isLevelEnabled(level);
}

inline void addTarget(std::shared_ptr<LogTarget> target) {
    Logger::getInstance().addTarget(target);
}

inline void removeTarget(std::shared_ptr<LogTarget> target) {
    Logger::getInstance().removeTarget(target);
}

inline void clearTargets() {
    Logger::getInstance().clearTargets();
}

inline void log(Level level, const std::string& message) {
    Logger::getInstance().log(level, message);
}

inline void flush() {
    Logger::getInstance().flush();
}

} // namespace logging

// 便捷宏
#define LOG_TRACE(message) if (logging::isLevelEnabled(logging::Level::TRACE)) logging::log(logging::Level::TRACE, message)
#define LOG_DEBUG(message) if (logging::isLevelEnabled(logging::Level::DEBUG)) logging::log(logging::Level::DEBUG, message)
#define LOG_INFO(message)  if (logging::isLevelEnabled(logging::Level::INFO))  logging::log(logging::Level::INFO, message)
#define LOG_WARN(message)  if (logging::isLevelEnabled(logging::Level::WARNING)) logging::log(logging::Level::WARNING, message)
#define LOG_ERROR(message) if (logging::isLevelEnabled(logging::Level::ERROR)) logging::log(logging::Level::ERROR, message)
#define LOG_FATAL(message) if (logging::isLevelEnabled(logging::Level::FATAL)) logging::log(logging::Level::FATAL, message)
```

## 使用示例

```cpp
// 初始化日志系统
void initLogging() {
    // 设置日志级别
    logging::setLevel(logging::Level::DEBUG);

    // 添加文件日志目标
    auto fileTarget = std::make_shared<logging::FileTarget>("sdk.log");
    logging::addTarget(fileTarget);

    LOG_INFO("日志系统初始化完成");
}

// 在网络模块中使用
void AsioNetworkModel::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        if (error != boost::asio::error::operation_aborted) {
            LOG_ERROR("接收数据错误: " + error.message());
            disconnect();
        }
        return;
    }

    LOG_DEBUG("接收到 " + std::to_string(bytes_transferred) + " 字节数据");
    // ...
}

// 在协议解析模块中使用
std::unique_ptr<IMessage> X30Protocol::parseReceivedData(const std::string& data) {
    try {
        // ...
        LOG_DEBUG("成功解析消息，类型: " + std::to_string(static_cast<int>(type)));
        return message;
    } catch (const std::exception& e) {
        LOG_ERROR("解析数据异常: " + std::string(e.what()));
        return nullptr;
    }
}

// 在回调函数中使用
template<typename Callback, typename... Args>
void safeCallback(const Callback& callback, const std::string& callbackType, Args&&... args) {
    try {
        callback(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        LOG_ERROR(callbackType + " 回调函数异常: " + std::string(e.what()));
    } catch (...) {
        LOG_ERROR(callbackType + " 回调函数发生未知异常");
    }
}
```

## 集成到 SDK 中

要将日志系统集成到 SDK 中，需要进行以下步骤：

1. **创建日志模块**：实现上述日志系统
2. **修改现有代码**：将现有的 `std::cerr` 和 `std::cout` 替换为日志宏
3. **添加配置选项**：在 `SdkOptions` 中添加日志配置选项
4. **初始化日志系统**：在 SDK 初始化时初始化日志系统
5. **提供日志回调接口**：允许用户自定义日志处理

```cpp
// 在 SdkOptions 中添加日志配置
struct SdkOptions {
    // 现有选项...

    // 日志选项
    bool enableLogging = true;
    logging::Level logLevel = logging::Level::INFO;
    std::string logFile;
    bool logToConsole = true;
    bool logToFile = false;
};

// 在 NavigationSdk 类中添加日志方法
class NavigationSdk {
public:
    // 现有方法...

    // 设置日志级别
    void setLogLevel(logging::Level level);

    // 设置日志回调
    using LogCallback = std::function<void(logging::Level, const std::string&)>;
    void setLogCallback(LogCallback callback);

    // 启用/禁用日志
    void enableLogging(bool enable);

    // 设置日志文件
    void setLogFile(const std::string& filename);

private:
    // 实现...
};
```

## 优先级建议

根据当前 SDK 的状态，建议按以下优先级实现日志系统：

1. **高优先级**：
   - 实现基本的日志接口和级别控制
   - 替换现有的 `std::cerr` 和 `std::cout`
   - 添加文件日志支持

2. **中优先级**：
   - 实现线程安全的日志队列
   - 添加上下文信息
   - 提供日志回调接口

3. **低优先级**：
   - 实现异步日志
   - 添加更多日志目标（如网络、系统日志等）
   - 性能优化

## 总结

一个完善的日志系统对于 SDK 的健壮性和可维护性至关重要。通过实现本文档中建议的日志系统，可以显著提高 SDK 的调试能力和问题排查效率，为用户提供更好的开发体验。

在实现过程中，应注意平衡日志系统的功能和性能，确保日志记录不会对 SDK 的核心功能造成显著影响。同时，应提供足够的配置选项，允许用户根据需要调整日志行为。
