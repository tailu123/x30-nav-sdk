#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <rapidxml/rapidxml.hpp>
#include <string>
#include <vector>

namespace protocol {

// 消息类型枚举
enum class MessageType {
    // 内部消息类型
    PROCEDURE_RESET = 0,

    // 请求消息类型
    GET_REAL_TIME_STATUS_REQ = 1002,
    NAVIGATION_TASK_REQ = 1003,
    CANCEL_TASK_REQ = 1004,
    QUERY_STATUS_REQ = 1007,

    // 响应消息类型
    GET_REAL_TIME_STATUS_RESP = 2002,
    NAVIGATION_TASK_RESP = 2003,
    CANCEL_TASK_RESP = 2004,
    QUERY_STATUS_RESP = 2007,
};

// 错误码枚举
enum class ErrorCode { SUCCESS = 0, FAILURE = 1, CANCELLED = 2 };

// 导航任务状态枚举
enum class NavigationStatus {
    COMPLETED = 0,  // 执行完成
    EXECUTING = 1,  // 正在执行
    FAILED = -1     // 无法执行
};

// 打印NavigationStatus
std::ostream& operator<<(std::ostream& os, const NavigationStatus& status);

// 导航点信息
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

    // 从JSON对象加载数据
    static NavigationPoint fromJson(const nlohmann::json& j) {
        NavigationPoint point;
        point.mapId = j.value("MapID", 0);
        point.value = j.value("Value", 0);
        point.posX = j.value("PosX", 0.0);
        point.posY = j.value("PosY", 0.0);
        point.posZ = j.value("PosZ", 0.0);
        point.angleYaw = j.value("AngleYaw", 0.0);
        point.pointInfo = j.value("PointInfo", 0);
        point.gait = j.value("Gait", 0);
        point.speed = j.value("Speed", 0);
        point.manner = j.value("Manner", 0);
        point.obsMode = j.value("ObsMode", 0);
        point.navMode = j.value("NavMode", 0);
        point.terrain = j.value("Terrain", 0);
        point.posture = j.value("Posture", 0);
        return point;
    }
};

// 打印NavigationPoint
std::ostream& operator<<(std::ostream& os, const NavigationPoint& point);

// 基础消息接口
class IMessage {
public:
    virtual ~IMessage() = default;
    std::string serialize() const;

    virtual bool deserialize(const std::string& xml) = 0;
    virtual MessageType getType() const = 0;

protected:
    virtual std::string serializeToXml() const = 0;
};

// =============== 请求消息定义 ===============

// 导航任务请求消息

// 获取实时基础状态请求消息
class GetRealTimeStatusRequest : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::GET_REAL_TIME_STATUS_REQ;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::string timestamp;
};

class NavigationTaskRequest : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::NAVIGATION_TASK_REQ;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::vector<NavigationPoint> points;
    std::string timestamp;
};

// 取消任务请求消息
class CancelTaskRequest : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::CANCEL_TASK_REQ;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::string timestamp;
};

// 查询状态请求消息
class QueryStatusRequest : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::QUERY_STATUS_REQ;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::string timestamp;
};

// =============== 响应消息定义 ===============

// 获取实时基础状态响应消息
class GetRealTimeStatusResponse : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::GET_REAL_TIME_STATUS_RESP;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    int motionState;                // 运动状态
    double posX;                    // 位置X
    double posY;                    // 位置Y
    double posZ;                    // 位置Z
    double angleYaw;                // 角度Yaw
    double roll;                    // 角度Roll
    double pitch;                   // 角度Pitch
    double yaw;                     // 角度Yaw
    double speed;                   // 速度
    double curOdom;                 // 当前里程
    double sumOdom;                 // 累计里程
    unsigned long long curRuntime;  // 当前运行时间
    unsigned long long sumRuntime;  // 累计运行时间
    double res;                     // 响应时间
    double x0;                      // 坐标X0
    double y0;                      // 坐标Y0
    int h;                          // 高度
    int electricity;                // 电量
    int location;                   // 位置  定位正常=0, 定位丢失=1
    int RTKState;                   // RTK状态
    int onDockState;                // 上岸状态
    int gaitState;                  // 步态状态
    int motorState;                 // 电机状态
    int chargeState;                // 充电状态
    int controlMode;                // 控制模式
    int mapUpdateState;             // 地图更新状态

    std::string timestamp;
};

// 导航任务响应消息
class NavigationTaskResponse : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::NAVIGATION_TASK_RESP;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    int value;            // 任务值
    ErrorCode errorCode;  // 错误码: 成功=0，失败=1，取消=2
    int errorStatus;      // 错误状态码，参考错误状态表
    std::string timestamp;
};

// 取消任务响应消息
class CancelTaskResponse : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::CANCEL_TASK_RESP;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    ErrorCode errorCode;  // 错误码: 成功=0，失败=1
    std::string timestamp;
};

// 查询状态响应消息
class QueryStatusResponse : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::QUERY_STATUS_RESP;
    }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    int value;                // 状态值
    NavigationStatus status;  // 导航任务执行状态
    ErrorCode errorCode;      // 错误码
    std::string timestamp;
};

// =============== 内部消息定义 ===============
// 导航任务重置请求消息
class ProcedureReset : public IMessage {
public:
    MessageType getType() const override {
        return MessageType::PROCEDURE_RESET;
    }
    std::string serializeToXml() const override {
        return "";
    };
    bool deserialize(const std::string&) override {
        return true;
    };
};

// 消息工厂
class MessageFactory {
public:
    static std::unique_ptr<IMessage> createMessage(MessageType type);
    static std::unique_ptr<IMessage> parseMessage(const std::string& xml);

private:
    static std::unique_ptr<IMessage> createRequestMessage(MessageType type);
    static std::unique_ptr<IMessage> createResponseMessage(MessageType type);
};

// 错误状态码定义
struct ErrorStatus {
    static constexpr int DEFAULT = 0;             // 异常码默认值
    static constexpr int TASK_CANCELLED = 8962;   // 单点巡检任务被取消
    static constexpr int TASK_COMPLETED = 8960;   // 单点巡检任务执行完成
    static constexpr int MOTION_ERROR = 41729;    // 运动状态异常，任务失败(软急停、摔倒)
    static constexpr int LOW_BATTERY = 41730;     // 电量过低，任务失败
    static constexpr int MOTOR_OVERHEAT = 41731;  // 电机过温异常，任务失败
    static constexpr int CHARGING = 41732;        // 正在使用充电器充电，任务失败
                                                  // ... 其他错误状态码定义
};

}  // namespace protocol
