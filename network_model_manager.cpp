#include "network/network_model_manager.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include "common/utils.hpp"
#include "network/base_network_model.hpp"
namespace network {

// NetworkModelManager实现
NetworkModelManager::NetworkModelManager(network::NetworkModelType model_type, common::MessageQueue& message_queue)
    : model_type_(model_type), message_queue_(message_queue) {
}

NetworkModelManager::~NetworkModelManager() {
    stop();
}

bool NetworkModelManager::start(const std::string& host, uint16_t port) {
    try {
        network_model_ = network::NetworkModelFactory::createNetworkModel(model_type_, message_queue_);

        return network_model_->connect(host, port);
    }
    catch (const std::exception& e) {
        spdlog::error("[{}]: [NetworkModelManager]: 启动失败, 错误: {}", common::getCurrentTimestamp(), e.what());
        // std::cout << "[NetworkModelManager]: 启动失败, 错误: " << e.what() << std::endl;
        return false;
    }
}

void NetworkModelManager::stop() {
    network_model_->disconnect();
}

std::shared_ptr<BaseNetworkModel> NetworkModelManager::getNetworkModel() {
    return network_model_;
}

}  // namespace network
