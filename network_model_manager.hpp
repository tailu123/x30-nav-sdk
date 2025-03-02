#pragma once

#include "common/message_queue.hpp"
#include "network/i_network_model_manager.hpp"
#include "network/network_model_factory.hpp"

namespace network {

// 通信管理器
class NetworkModelManager : public INetworkModelManager {
public:
    explicit NetworkModelManager(network::NetworkModelType model_type, common::MessageQueue& message_queue);
    ~NetworkModelManager() override;

    // 启动和停止
    bool start(const std::string& host, uint16_t port) override;
    void stop() override;

    // 获取网络模型实例
    std::shared_ptr<BaseNetworkModel> getNetworkModel() override;

private:
    network::NetworkModelType model_type_;
    std::shared_ptr<BaseNetworkModel> network_model_;
    common::MessageQueue& message_queue_;
};

}  // namespace network
