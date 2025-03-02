#pragma once
#include <memory>

namespace common {
class MessageQueue;
}  // namespace common

namespace network {

class BaseNetworkModel;
enum class NetworkModelType { ASIO, EPOLL, LIBHV };

class NetworkModelFactory {
public:
    static std::shared_ptr<BaseNetworkModel> createNetworkModel(NetworkModelType type,
                                                                common::MessageQueue& message_queue);
};

}  // namespace network
