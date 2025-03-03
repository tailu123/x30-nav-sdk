#pragma once

#include <dog_navigation/types.h>
#include <memory>
#include <string>
#include <future>

namespace dog_navigation {

// 前向声明，隐藏实现细节
class NavigationSdkImpl;

/**
 * @brief X30机器狗导航SDK主类
 *
 * 该类提供了与X30机器狗进行通信的主要接口，包括连接管理、
 * 导航任务控制、状态查询等功能。
 */
class NavigationSdk {
public:
    /**
     * @brief 构造函数
     * @param options SDK配置选项
     */
    explicit NavigationSdk(const SdkOptions& options = SdkOptions());

    /**
     * @brief 析构函数
     */
    ~NavigationSdk();

    /**
     * @brief 禁用拷贝构造函数
     */
    NavigationSdk(const NavigationSdk&) = delete;

    /**
     * @brief 禁用赋值操作符
     */
    NavigationSdk& operator=(const NavigationSdk&) = delete;

    /**
     * @brief 连接到机器狗控制系统
     * @param host 主机地址
     * @param port 端口号
     * @return 连接是否成功
     */
    bool connect(const std::string& host, uint16_t port);

    /**
     * @brief 断开与机器狗控制系统的连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 设置事件回调函数
     * @param callback 事件回调函数, 用于网路连接状态通知
     */
    void setEventCallback(EventCallback callback);

    /**
     * @brief 获取机器狗的实时状态
     * @return 实时状态信息
     */
    RealTimeStatus getRealTimeStatus();

    /**
     * @brief 基于回调的异步开始导航任务
     * @param points 导航点列表
     * @param callback 导航结果回调函数
     */
    void startNavigationAsync(const std::vector<NavigationPoint>& points, NavigationResultCallback callback);

    /**
     * @brief 取消当前导航任务
     * @return 操作是否成功
     */
    bool cancelNavigation();

    /**
     * @brief 查询当前导航任务状态
     * @return 任务状态查询结果
     */
    TaskStatusResult queryTaskStatus();

    /**
     * @brief 获取SDK版本
     * @return SDK版本字符串
     */
    static std::string getVersion();

private:
    std::unique_ptr<NavigationSdkImpl> impl_; ///< PIMPL实现
};

} // namespace dog_navigation
