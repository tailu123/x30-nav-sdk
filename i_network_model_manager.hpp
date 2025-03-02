#pragma once

#include <memory>

namespace network {

class BaseNetworkModel;

// 通信管理器
class INetworkModelManager {
public:
    virtual ~INetworkModelManager() = default;

    // 启动和停止
    virtual bool start(const std::string&, uint16_t) = 0;
    virtual void stop() = 0;

    // 获取网络模型实例
    virtual std::shared_ptr<BaseNetworkModel> getNetworkModel() = 0;
};

}  // namespace network
