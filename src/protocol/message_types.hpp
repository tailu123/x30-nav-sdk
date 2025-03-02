#pragma once

#include "i_message.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <rapidxml/rapidxml.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace protocol {

/**
 * @brief 导航点信息
 */
struct NavigationPoint {
    int mapId = 0;
    int value = 0;
    double posX = 0.0;
    double posY = 0.0;
    double posZ = 0.0;
    double angleYaw = 0.0;
    int pointInfo = 0;
    int gait = 0;
    int speed = 0;
    int manner = 0;
    int obsMode = 0;
    int navMode = 0;
    int terrain = 0;
    int posture = 0;
};

/**
 * @brief 获取当前时间戳字符串
 * @return 格式化的时间戳字符串
 */
inline std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/**
 * @brief 获取实时状态请求
 */
class GetRealTimeStatusRequest : public IMessage {
public:
    std::string timestamp;

    GetRealTimeStatusRequest() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::GET_REAL_TIME_STATUS_REQ;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1002</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items/>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);
                if (j.contains("timestamp")) {
                    timestamp = j["timestamp"];
                }
                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 获取实时状态响应
 */
class GetRealTimeStatusResponse : public IMessage {
public:
    int motionState = 0;
    double posX = 0.0;
    double posY = 0.0;
    double posZ = 0.0;
    double angleYaw = 0.0;
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
    double speed = 0.0;
    double curOdom = 0.0;
    double sumOdom = 0.0;
    uint64_t curRuntime = 0;
    uint64_t sumRuntime = 0;
    double res = 0.0;
    double x0 = 0.0;
    double y0 = 0.0;
    int h = 0;
    int electricity = 0;
    int location = 0;
    int RTKState = 0;
    int onDockState = 0;
    int gaitState = 0;
    int motorState = 0;
    int chargeState = 0;
    int controlMode = 0;
    int mapUpdateState = 0;
    std::string timestamp;

    GetRealTimeStatusResponse() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::GET_REAL_TIME_STATUS_RESP;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "  <Type>1002</Type>\n";
        ss << "  <Command>1</Command>\n";
        ss << "  <Time>" << timestamp << "</Time>\n";
        ss << "  <Items>\n";
        ss << "    <MotionState>" << motionState << "</MotionState>\n";
        ss << "    <PosX>" << posX << "</PosX>\n";
        ss << "    <PosY>" << posY << "</PosY>\n";
        ss << "    <PosZ>" << posZ << "</PosZ>\n";
        ss << "    <AngleYaw>" << angleYaw << "</AngleYaw>\n";
        ss << "    <Roll>" << roll << "</Roll>\n";
        ss << "    <Pitch>" << pitch << "</Pitch>\n";
        ss << "    <Yaw>" << yaw << "</Yaw>\n";
        ss << "    <Speed>" << speed << "</Speed>\n";
        ss << "    <CurOdom>" << curOdom << "</CurOdom>\n";
        ss << "    <SumOdom>" << sumOdom << "</SumOdom>\n";
        ss << "    <CurRuntime>" << curRuntime << "</CurRuntime>\n";
        ss << "    <SumRuntime>" << sumRuntime << "</SumRuntime>\n";
        ss << "    <Res>" << res << "</Res>\n";
        ss << "    <X0>" << x0 << "</X0>\n";
        ss << "    <Y0>" << y0 << "</Y0>\n";
        ss << "    <H>" << h << "</H>\n";
        ss << "    <Electricity>" << electricity << "</Electricity>\n";
        ss << "    <Location>" << location << "</Location>\n";
        ss << "    <RTKState>" << RTKState << "</RTKState>\n";
        ss << "    <OnDockState>" << onDockState << "</OnDockState>\n";
        ss << "    <GaitState>" << gaitState << "</GaitState>\n";
        ss << "    <MotorState>" << motorState << "</MotorState>\n";
        ss << "    <ChargeState>" << chargeState << "</ChargeState>\n";
        ss << "    <ControlMode>" << controlMode << "</ControlMode>\n";
        ss << "    <MapUpdateState>" << mapUpdateState << "</MapUpdateState>\n";
        ss << "  </Items>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                rapidxml::xml_node<>* items_node = root->first_node("Items");
                if (!items_node) return false;

                // 解析各个字段
                auto get_node_value = [&](const char* name, auto& value) {
                    rapidxml::xml_node<>* node = items_node->first_node(name);
                    if (node) {
                        std::stringstream ss(node->value());
                        ss >> value;
                    }
                };

                get_node_value("MotionState", motionState);
                get_node_value("PosX", posX);
                get_node_value("PosY", posY);
                get_node_value("PosZ", posZ);
                get_node_value("AngleYaw", angleYaw);
                get_node_value("Roll", roll);
                get_node_value("Pitch", pitch);
                get_node_value("Yaw", yaw);
                get_node_value("Speed", speed);
                get_node_value("CurOdom", curOdom);
                get_node_value("SumOdom", sumOdom);
                get_node_value("CurRuntime", curRuntime);
                get_node_value("SumRuntime", sumRuntime);
                get_node_value("Res", res);
                get_node_value("X0", x0);
                get_node_value("Y0", y0);
                get_node_value("H", h);
                get_node_value("Electricity", electricity);
                get_node_value("Location", location);
                get_node_value("RTKState", RTKState);
                get_node_value("OnDockState", onDockState);
                get_node_value("GaitState", gaitState);
                get_node_value("MotorState", motorState);
                get_node_value("ChargeState", chargeState);
                get_node_value("ControlMode", controlMode);
                get_node_value("MapUpdateState", mapUpdateState);

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                motionState = j["motionState"];
                posX = j["posX"];
                posY = j["posY"];
                posZ = j["posZ"];
                angleYaw = j["angleYaw"];
                roll = j["roll"];
                pitch = j["pitch"];
                yaw = j["yaw"];
                speed = j["speed"];
                curOdom = j["curOdom"];
                sumOdom = j["sumOdom"];
                curRuntime = j["curRuntime"];
                sumRuntime = j["sumRuntime"];
                res = j["res"];
                x0 = j["x0"];
                y0 = j["y0"];
                h = j["h"];
                electricity = j["electricity"];
                location = j["location"];
                RTKState = j["RTKState"];
                onDockState = j["onDockState"];
                gaitState = j["gaitState"];
                motorState = j["motorState"];
                chargeState = j["chargeState"];
                controlMode = j["controlMode"];
                mapUpdateState = j["mapUpdateState"];
                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 导航任务请求
 */
class NavigationTaskRequest : public IMessage {
public:
    std::vector<NavigationPoint> points;
    std::string timestamp;

    NavigationTaskRequest() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::NAVIGATION_TASK_REQ;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1003</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";

        // 添加导航点
        for (const auto& point : points) {
            ss << "<Items>\n";
            ss << "  <MapId>" << point.mapId << "</MapId>\n";
            ss << "  <Value>" << point.value << "</Value>\n";
            ss << "  <PosX>" << point.posX << "</PosX>\n";
            ss << "  <PosY>" << point.posY << "</PosY>\n";
            ss << "  <PosZ>" << point.posZ << "</PosZ>\n";
            ss << "  <AngleYaw>" << point.angleYaw << "</AngleYaw>\n";
            ss << "  <PointInfo>" << point.pointInfo << "</PointInfo>\n";
            ss << "  <Gait>" << point.gait << "</Gait>\n";
            ss << "  <Speed>" << point.speed << "</Speed>\n";
            ss << "  <Manner>" << point.manner << "</Manner>\n";
            ss << "  <ObsMode>" << point.obsMode << "</ObsMode>\n";
            ss << "  <NavMode>" << point.navMode << "</NavMode>\n";
            ss << "  <Terrain>" << point.terrain << "</Terrain>\n";
            ss << "  <Posture>" << point.posture << "</Posture>\n";
            ss << "</Items>\n";
        }

        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                // 清空现有点
                points.clear();

                // 解析所有Items节点
                for (rapidxml::xml_node<>* items_node = root->first_node("Items");
                     items_node;
                     items_node = items_node->next_sibling("Items")) {

                    NavigationPoint point;

                    // 解析各个字段
                    auto get_node_value = [&](const char* name, auto& value) {
                        rapidxml::xml_node<>* node = items_node->first_node(name);
                        if (node) {
                            std::stringstream ss(node->value());
                            ss >> value;
                        }
                    };

                    get_node_value("MapId", point.mapId);
                    get_node_value("Value", point.value);
                    get_node_value("PosX", point.posX);
                    get_node_value("PosY", point.posY);
                    get_node_value("PosZ", point.posZ);
                    get_node_value("AngleYaw", point.angleYaw);
                    get_node_value("PointInfo", point.pointInfo);
                    get_node_value("Gait", point.gait);
                    get_node_value("Speed", point.speed);
                    get_node_value("Manner", point.manner);
                    get_node_value("ObsMode", point.obsMode);
                    get_node_value("NavMode", point.navMode);
                    get_node_value("Terrain", point.terrain);
                    get_node_value("Posture", point.posture);

                    points.push_back(point);
                }

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                points.clear();

                if (j.contains("points") && j["points"].is_array()) {
                    for (const auto& point_json : j["points"]) {
                        NavigationPoint point;
                        point.mapId = point_json["mapId"];
                        point.value = point_json["value"];
                        point.posX = point_json["posX"];
                        point.posY = point_json["posY"];
                        point.posZ = point_json["posZ"];
                        point.angleYaw = point_json["angleYaw"];
                        point.pointInfo = point_json["pointInfo"];
                        point.gait = point_json["gait"];
                        point.speed = point_json["speed"];
                        point.manner = point_json["manner"];
                        point.obsMode = point_json["obsMode"];
                        point.navMode = point_json["navMode"];
                        point.terrain = point_json["terrain"];
                        point.posture = point_json["posture"];
                        points.push_back(point);
                    }
                }

                if (j.contains("timestamp")) {
                    timestamp = j["timestamp"];
                }

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 导航任务响应
 */
class NavigationTaskResponse : public IMessage {
public:
    int value = 0;
    ErrorCode errorCode = ErrorCode::SUCCESS;
    int errorStatus = 0;
    std::string timestamp;

    NavigationTaskResponse() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::NAVIGATION_TASK_RESP;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1003</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << "  <Value>" << value << "</Value>\n";
        ss << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
        ss << "  <ErrorStatus>" << errorStatus << "</ErrorStatus>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                rapidxml::xml_node<>* items_node = root->first_node("Items");
                if (!items_node) return false;

                // 解析各个字段
                auto get_node_value = [&](const char* name, auto& value) {
                    rapidxml::xml_node<>* node = items_node->first_node(name);
                    if (node) {
                        std::stringstream ss(node->value());
                        ss >> value;
                    }
                };

                get_node_value("Value", value);

                // 解析错误码
                int error_code_value = 0;
                get_node_value("ErrorCode", error_code_value);
                errorCode = static_cast<ErrorCode>(error_code_value);

                get_node_value("ErrorStatus", errorStatus);

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                value = j["value"];
                errorCode = static_cast<ErrorCode>(static_cast<int>(j["errorCode"]));
                errorStatus = j["errorStatus"];
                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 查询任务状态请求
 */
class QueryStatusRequest : public IMessage {
public:
    std::string timestamp;

    QueryStatusRequest() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::QUERY_STATUS_REQ;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1007</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items/>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 查询任务状态响应
 */
class QueryStatusResponse : public IMessage {
public:
    int value = 0;
    NavigationStatus status = NavigationStatus::COMPLETED;
    ErrorCode errorCode = ErrorCode::SUCCESS;
    std::string timestamp;

    QueryStatusResponse() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::QUERY_STATUS_RESP;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1007</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << "  <Value>" << value << "</Value>\n";
        ss << "  <Status>" << static_cast<int>(status) << "</Status>\n";
        ss << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                rapidxml::xml_node<>* items_node = root->first_node("Items");
                if (!items_node) return false;

                // 解析各个字段
                auto get_node_value = [&](const char* name, auto& value) {
                    rapidxml::xml_node<>* node = items_node->first_node(name);
                    if (node) {
                        std::stringstream ss(node->value());
                        ss >> value;
                    }
                };

                get_node_value("Value", value);

                // 解析状态
                int status_value = 0;
                get_node_value("Status", status_value);
                status = static_cast<NavigationStatus>(status_value);

                // 解析错误码
                int error_code_value = 0;
                get_node_value("ErrorCode", error_code_value);
                errorCode = static_cast<ErrorCode>(error_code_value);

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                value = j["value"];
                status = static_cast<NavigationStatus>(static_cast<int>(j["status"]));
                if (j.contains("errorCode")) {
                    errorCode = static_cast<ErrorCode>(static_cast<int>(j["errorCode"]));
                }
                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 取消任务请求
 */
class CancelTaskRequest : public IMessage {
public:
    std::string timestamp;

    CancelTaskRequest() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::CANCEL_TASK_REQ;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1004</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items/>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

/**
 * @brief 取消任务响应
 */
class CancelTaskResponse : public IMessage {
public:
    ErrorCode errorCode = ErrorCode::SUCCESS;
    std::string timestamp;

    CancelTaskResponse() : timestamp(getCurrentTimestamp()) {}

    MessageType getType() const override {
        return MessageType::CANCEL_TASK_RESP;
    }

    std::string serialize() const override {
        // 使用XML格式
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1004</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";
        return ss.str();
    }

    bool deserialize(const std::string& data) override {
        try {
            // 检查是否为XML格式
            if (data.find("<?xml") != std::string::npos || data.find("<PatrolDevice>") != std::string::npos) {
                rapidxml::xml_document<> doc;
                std::vector<char> buffer(data.begin(), data.end());
                buffer.push_back('\0');
                doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                if (!root) return false;

                rapidxml::xml_node<>* time_node = root->first_node("Time");
                if (time_node) {
                    timestamp = time_node->value();
                }

                rapidxml::xml_node<>* items_node = root->first_node("Items");
                if (!items_node) return false;

                // 解析错误码
                int error_code_value = 0;
                rapidxml::xml_node<>* error_code_node = items_node->first_node("ErrorCode");
                if (error_code_node) {
                    std::stringstream ss(error_code_node->value());
                    ss >> error_code_value;
                    errorCode = static_cast<ErrorCode>(error_code_value);
                }

                return true;
            } else {
                // 兼容JSON格式
                auto j = nlohmann::json::parse(data);

                errorCode = static_cast<ErrorCode>(static_cast<int>(j["errorCode"]));
                timestamp = j["timestamp"];

                return true;
            }
        } catch (const std::exception& e) {
            return false;
        }
    }
};

} // namespace protocol
