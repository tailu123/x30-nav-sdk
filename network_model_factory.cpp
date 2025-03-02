#include "network/network_model_factory.hpp"
#include "network/asio_network_model.hpp"
#include "network/epoll_network_model.hpp"
#include "network/libhv_network_model.hpp"

namespace network {

std::shared_ptr<BaseNetworkModel> NetworkModelFactory::createNetworkModel(NetworkModelType type,
                                                                          common::MessageQueue& message_queue) {
    switch (type) {
        case NetworkModelType::ASIO:
            return std::make_unique<AsioNetworkModel>(message_queue);

        case NetworkModelType::EPOLL:
            return std::make_unique<EpollNetworkModel>(message_queue);

        case NetworkModelType::LIBHV:
            return std::make_unique<LibhvNetworkModel>(message_queue);

        default:
            throw std::runtime_error("Unknown network model type");
    }
}

}  // namespace network
