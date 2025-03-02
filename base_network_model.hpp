#pragma once

#include <string>
#include "protocol/x30_protocol.hpp"

namespace network {

class BaseNetworkModel {
public:
    virtual ~BaseNetworkModel() = default;
    virtual bool connect(const std::string& host, uint16_t port) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual void sendMessage(const protocol::IMessage& message) = 0;
};
}  // namespace network
