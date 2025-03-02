#include "x30_protocol.hpp"
#include <nlohmann/json.hpp>
#include <rapidxml/rapidxml.hpp>
#include <sstream>
#include <iostream>

namespace protocol {

std::unique_ptr<IMessage> X30Protocol::parseReceivedData(const std::string& data) {
    try {
        // 检查数据长度是否足够包含协议头
        constexpr size_t HEADER_SIZE = sizeof(ProtocolHeader);
        if (data.size() < HEADER_SIZE) {
            std::cerr << "数据长度不足以包含协议头" << std::endl;
            return nullptr;
        }

        // 解析协议头
        const ProtocolHeader* header = reinterpret_cast<const ProtocolHeader*>(data.data());

        // 验证同步字节
        if (!header->validateSyncBytes()) {
            std::cerr << "协议头同步字节无效" << std::endl;
            return nullptr;
        }

        // 获取消息体长度
        uint16_t body_size = header->getBodySize();

        // 检查数据长度是否足够
        if (data.size() < HEADER_SIZE + body_size) {
            std::cerr << "数据长度不足: 期望 " << (HEADER_SIZE + body_size) << ", 实际 " << data.size() << std::endl;
            return nullptr;
        }

        // 提取消息体
        std::string message_body = data.substr(HEADER_SIZE, body_size);

        // 提取消息类型
        MessageType type = extractMessageType(message_body);

        // 创建对应类型的消息对象
        auto message = createMessage(type);
        if (!message) {
            return nullptr;
        }

        // 反序列化消息
        if (!message->deserialize(message_body)) {
            return nullptr;
        }

        return message;
    } catch (const std::exception& e) {
        std::cerr << "解析数据异常: " << e.what() << std::endl;
        return nullptr;
    }
}

std::string X30Protocol::serializeMessage(const IMessage& message) {
    // 获取消息体
    std::string message_body = message.serialize();

    // 创建协议头
    ProtocolHeader header(message_body.size());

    // 组合协议头和消息体
    std::string result;
    constexpr size_t HEADER_SIZE = sizeof(ProtocolHeader);
    result.reserve(HEADER_SIZE + message_body.size());
    result.append(reinterpret_cast<const char*>(&header), HEADER_SIZE);
    result.append(message_body);

    return result;
}

MessageType X30Protocol::extractMessageType(const std::string& data) {
    try {
        // 检查数据是否为XML格式
        if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
            // 提取Type字段
            int type = extractTypeFromXml(data);

            // 根据Type确定消息类型
            return determineMessageType(type);
        }

        // 如果不是XML格式，尝试解析为JSON格式（兼容旧代码）
        auto j = nlohmann::json::parse(data);

        // 根据消息内容判断类型
        if (j.contains("motionState") && j.contains("posX")) {
            return MessageType::GET_REAL_TIME_STATUS_RESP;
        } else if (j.contains("points")) {
            return MessageType::NAVIGATION_TASK_REQ;
        } else if (j.contains("value") && j.contains("errorCode") && j.contains("errorStatus")) {
            return MessageType::NAVIGATION_TASK_RESP;
        } else if (j.contains("value") && j.contains("status")) {
            return MessageType::QUERY_STATUS_RESP;
        } else if (j.contains("errorCode") && !j.contains("value")) {
            return MessageType::CANCEL_TASK_RESP;
        } else if (j.contains("timestamp") && j.size() == 1) {
            // 这里无法区分几种只有timestamp的请求，默认返回获取实时状态请求
            return MessageType::GET_REAL_TIME_STATUS_REQ;
        }

        return MessageType::UNKNOWN;
    } catch (const std::exception& e) {
        std::cerr << "提取消息类型异常: " << e.what() << std::endl;
        return MessageType::UNKNOWN;
    }
}

int X30Protocol::extractTypeFromXml(const std::string& data) {
    try {
        rapidxml::xml_document<> doc;
        // 创建一个可修改的数据副本，因为rapidxml会修改输入字符串
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');

        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        // 获取根节点
        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) {
            return 0;
        }

        // 获取Type节点
        rapidxml::xml_node<>* type_node = root->first_node("Type");
        if (!type_node) {
            return 0;
        }

        // 转换Type值为整数
        return std::stoi(type_node->value());
    } catch (const std::exception& e) {
        std::cerr << "提取XML Type异常: " << e.what() << std::endl;
        return 0;
    }
}

int X30Protocol::extractCommandFromXml(const std::string& data) {
    try {
        rapidxml::xml_document<> doc;
        // 创建一个可修改的数据副本
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');

        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        // 获取根节点
        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) {
            return 0;
        }

        // 获取Command节点
        rapidxml::xml_node<>* command_node = root->first_node("Command");
        if (!command_node) {
            return 0;
        }

        // 转换Command值为整数
        return std::stoi(command_node->value());
    } catch (const std::exception& e) {
        std::cerr << "提取XML Command异常: " << e.what() << std::endl;
        return 0;
    }
}

MessageType X30Protocol::determineMessageType(int type) {
    // 查找Type对应的消息类型
    auto it = type_to_message_type_.find(type);
    if (it == type_to_message_type_.end()) {
        return MessageType::UNKNOWN;
    }

    // 检查是否有Items节点，如果没有或为空，则为请求消息
    // 由于请求和响应的 command 都是 1，不再使用 command 进行判断
    // 这里简化处理，根据 XML 中是否包含 Items 节点来判断是请求还是响应
    // 实际应用中可能需要更复杂的逻辑

    // 默认返回请求消息类型
    return it->second.first;
}

} // namespace protocol
