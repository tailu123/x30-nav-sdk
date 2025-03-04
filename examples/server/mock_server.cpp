#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <random>
#include <atomic>
#include <iomanip>
#include <rapidxml/rapidxml.hpp>

using boost::asio::ip::tcp;

// 前向声明
std::string getCurrentTimestamp();
std::string handleGetRealTimeStatusRequestXml(const std::string& request_data);
std::string handleNavigationTaskRequestXml(const std::string& request_data);
std::string handleQueryStatusRequestXml(const std::string& request_data);
std::string handleCancelTaskRequestXml(const std::string& request_data);

// 模拟服务器类
class MockServer {
public:
    MockServer(uint16_t port)
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
          running_(false) {
        // 设置地址重用选项
        boost::asio::socket_base::reuse_address option(true);
        acceptor_.set_option(option);
    }

    ~MockServer() {
        stop();
    }

    void start() {
        if (running_) {
            return;
        }

        running_ = true;
        std::cout << "模拟服务器已启动，监听端口: " << acceptor_.local_endpoint().port() << std::endl;

        // 启动接受连接
        startAccept();

        // 启动IO线程
        io_thread_ = std::thread([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                std::cerr << "IO线程异常: " << e.what() << std::endl;
            }
        });
    }

    void stop() {
        if (!running_) {
            return;
        }

        running_ = false;
        io_context_.stop();

        if (io_thread_.joinable()) {
            io_thread_.join();
        }

        std::cout << "模拟服务器已停止" << std::endl;
    }

private:
    void startAccept() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
            if (!error) {
                std::cout << "接受新连接: " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << std::endl;

                // 启动会话
                startSession(socket);
            } else {
                std::cerr << "接受连接错误: " << error.message() << std::endl;
            }

            // 继续接受下一个连接
            if (running_) {
                startAccept();
            }
        });
    }

    void startSession(std::shared_ptr<tcp::socket> socket) {
        auto session = std::make_shared<Session>(socket);
        session->start();
    }

    // 会话类
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(std::shared_ptr<tcp::socket> socket)
            : socket_(socket),
              receive_buffer_() {
        }

        void start() {
            startReceive();
        }

    private:
        void startReceive() {
            auto self = shared_from_this();
            socket_->async_read_some(
                boost::asio::buffer(receive_buffer_),
                [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (!error) {
                        // 处理接收到的数据
                        std::string data(receive_buffer_.data(), bytes_transferred);
                        handleRequest(data);

                        // 继续接收
                        startReceive();
                    } else if (error != boost::asio::error::operation_aborted) {
                        std::cerr << "接收数据错误: " << error.message() << std::endl;
                    }
                }
            );
        }

        void handleRequest(const std::string& request_data) {
            try {
                std::string response_data;

                // 检查是否为 XML 格式
                if (request_data.find("<?xml") != std::string::npos || request_data.find("<PatrolDevice>") != std::string::npos) {
                    // 解析 XML
                    rapidxml::xml_document<> doc;
                    std::vector<char> buffer(request_data.begin(), request_data.end());
                    buffer.push_back('\0');
                    doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

                    rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
                    if (!root) {
                        std::cerr << "无效的 XML 请求" << std::endl;
                        return;
                    }

                    rapidxml::xml_node<>* type_node = root->first_node("Type");
                    if (!type_node) {
                        std::cerr << "XML 请求缺少 Type 字段" << std::endl;
                        return;
                    }

                    int type = std::stoi(type_node->value());

                    // 根据 Type 处理不同类型的请求
                    switch (type) {
                        case 1002: // 获取实时状态
                            response_data = handleGetRealTimeStatusRequestXml(request_data);
                            break;
                        case 1003: // 导航任务
                            response_data = handleNavigationTaskRequestXml(request_data);
                            break;
                        case 1004: // 取消任务
                            response_data = handleCancelTaskRequestXml(request_data);
                            break;
                        case 1007: // 查询任务状态
                            response_data = handleQueryStatusRequestXml(request_data);
                            break;
                        default:
                            std::cerr << "未知的请求类型: " << type << std::endl;
                            return;
                    }
                } else {
                    // 尝试解析为 JSON 格式（兼容旧代码）
                    auto j = nlohmann::json::parse(request_data);

                    // 根据消息内容判断类型
                    if (j.contains("points")) {
                        response_data = handleNavigationTaskRequest(j);
                    } else if (j.contains("timestamp") && j.size() == 1) {
                        // 这里无法区分几种只有timestamp的请求，根据随机数决定
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> dis(0, 2);
                        int req_type = dis(gen);

                        switch (req_type) {
                            case 0:
                                response_data = handleGetRealTimeStatusRequest(j);
                                break;
                            case 1:
                                response_data = handleQueryStatusRequest(j);
                                break;
                            case 2:
                                response_data = handleCancelTaskRequest(j);
                                break;
                        }
                    }
                }

                if (!response_data.empty()) {
                    // 发送响应
                    sendResponse(response_data);
                }
            } catch (const std::exception& e) {
                std::cerr << "处理请求异常: " << e.what() << std::endl;
            }
        }

        std::string handleNavigationTaskRequest(const nlohmann::json&) {
            std::cout << "收到导航任务请求" << std::endl;

            // 生成响应
            nlohmann::json response;
            response["value"] = 1; // 目标点编号
            response["errorCode"] = 0; // 成功
            response["errorStatus"] = 0;
            response["timestamp"] = getCurrentTimestamp();

            return response.dump();
        }

        std::string handleGetRealTimeStatusRequest(const nlohmann::json&) {
            std::cout << "收到获取实时状态请求" << std::endl;

            // 生成随机状态
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> pos_dis(-100.0, 100.0);
            std::uniform_real_distribution<> angle_dis(0.0, 360.0);
            std::uniform_real_distribution<> speed_dis(0.0, 5.0);
            std::uniform_int_distribution<> electricity_dis(0, 100);

            // 生成响应
            nlohmann::json response;
            response["motionState"] = 1; // 运动中
            response["posX"] = pos_dis(gen);
            response["posY"] = pos_dis(gen);
            response["posZ"] = 0.0;
            response["angleYaw"] = angle_dis(gen);
            response["roll"] = 0.0;
            response["pitch"] = 0.0;
            response["yaw"] = angle_dis(gen);
            response["speed"] = speed_dis(gen);
            response["curOdom"] = 0.0;
            response["sumOdom"] = 0.0;
            response["curRuntime"] = 0;
            response["sumRuntime"] = 0;
            response["res"] = 0.0;
            response["x0"] = 0.0;
            response["y0"] = 0.0;
            response["h"] = 0;
            response["electricity"] = electricity_dis(gen);
            response["location"] = 0;
            response["RTKState"] = 0;
            response["onDockState"] = 0;
            response["gaitState"] = 0;
            response["motorState"] = 0;
            response["chargeState"] = 0;
            response["controlMode"] = 0;
            response["mapUpdateState"] = 0;
            response["timestamp"] = getCurrentTimestamp();

            return response.dump();
        }

        std::string handleQueryStatusRequest(const nlohmann::json&) {
            std::cout << "收到查询任务状态请求" << std::endl;

            // 生成随机状态
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> status_dis(0, 2);
            int status = status_dis(gen);
            if (status == 2) status = -1; // 失败状态

            // 生成响应
            nlohmann::json response;
            response["value"] = 1; // 目标点编号
            response["status"] = status;
            response["errorCode"] = 0;
            response["timestamp"] = getCurrentTimestamp();

            return response.dump();
        }

        std::string handleCancelTaskRequest(const nlohmann::json&) {
            std::cout << "收到取消任务请求" << std::endl;

            // 生成响应
            nlohmann::json response;
            response["errorCode"] = 0; // 成功
            response["timestamp"] = getCurrentTimestamp();

            return response.dump();
        }

        void sendResponse(const std::string& response_data) {
            auto self = shared_from_this();
            boost::asio::async_write(
                *socket_,
                boost::asio::buffer(response_data),
                [this, self](const boost::system::error_code& error, std::size_t) {
                    if (error) {
                        std::cerr << "发送响应错误: " << error.message() << std::endl;
                    }
                }
            );
        }

        std::string getCurrentTimestamp() const {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }

        std::shared_ptr<tcp::socket> socket_;
        std::array<char, 4096> receive_buffer_;
    };

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::thread io_thread_;
    std::atomic<bool> running_;
};

int main(int argc, char* argv[]) {
    try {
        // 检查命令行参数
        uint16_t port = 8080;
        if (argc > 1) {
            port = static_cast<uint16_t>(std::stoi(argv[1]));
        }

        // 创建并启动模拟服务器
        MockServer server(port);
        server.start();

        std::cout << "按回车键停止服务器..." << std::endl;
        std::cin.get();

        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// 添加 XML 格式的处理方法
std::string handleGetRealTimeStatusRequestXml(const std::string& request_data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(request_data.begin(), request_data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return "";

        rapidxml::xml_node<>* time_node = root->first_node("Time");
        std::string timestamp = time_node ? time_node->value() : getCurrentTimestamp();

        // 生成随机数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> pos_dis(-10.0, 10.0);
        std::uniform_real_distribution<> angle_dis(-3.14, 3.14);
        std::uniform_real_distribution<> speed_dis(0.0, 5.0);
        std::uniform_int_distribution<> int_dis(0, 100);

        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "  <Type>1002</Type>\n";
        ss << "  <Command>1</Command>\n";
        ss << "  <Time>" << timestamp << "</Time>\n";
        ss << "  <Items>\n";
        ss << "    <MotionState>" << int_dis(gen) % 5 << "</MotionState>\n";
        ss << "    <PosX>" << pos_dis(gen) << "</PosX>\n";
        ss << "    <PosY>" << pos_dis(gen) << "</PosY>\n";
        ss << "    <PosZ>" << pos_dis(gen) / 10.0 << "</PosZ>\n";
        ss << "    <AngleYaw>" << angle_dis(gen) << "</AngleYaw>\n";
        ss << "    <Roll>" << angle_dis(gen) / 10.0 << "</Roll>\n";
        ss << "    <Pitch>" << angle_dis(gen) / 10.0 << "</Pitch>\n";
        ss << "    <Yaw>" << angle_dis(gen) << "</Yaw>\n";
        ss << "    <Speed>" << speed_dis(gen) << "</Speed>\n";
        ss << "    <CurOdom>" << pos_dis(gen) + 10.0 << "</CurOdom>\n";
        ss << "    <SumOdom>" << pos_dis(gen) + 100.0 << "</SumOdom>\n";
        ss << "    <CurRuntime>" << int_dis(gen) * 100 << "</CurRuntime>\n";
        ss << "    <SumRuntime>" << int_dis(gen) * 10000 << "</SumRuntime>\n";
        ss << "    <Res>" << pos_dis(gen) / 100.0 + 0.1 << "</Res>\n";
        ss << "    <X0>" << pos_dis(gen) << "</X0>\n";
        ss << "    <Y0>" << pos_dis(gen) << "</Y0>\n";
        ss << "    <H>" << int_dis(gen) + 200 << "</H>\n";
        ss << "    <Electricity>" << int_dis(gen) << "</Electricity>\n";
        ss << "    <Location>" << int_dis(gen) % 2 << "</Location>\n";
        ss << "    <RTKState>" << int_dis(gen) % 2 << "</RTKState>\n";
        ss << "    <OnDockState>" << int_dis(gen) % 2 << "</OnDockState>\n";
        ss << "    <GaitState>" << int_dis(gen) % 3 << "</GaitState>\n";
        ss << "    <MotorState>" << int_dis(gen) % 2 << "</MotorState>\n";
        ss << "    <ChargeState>" << int_dis(gen) % 2 << "</ChargeState>\n";
        ss << "    <ControlMode>" << int_dis(gen) % 3 << "</ControlMode>\n";
        ss << "    <MapUpdateState>" << int_dis(gen) % 2 << "</MapUpdateState>\n";
        ss << "  </Items>\n";
        ss << "</PatrolDevice>";

        return ss.str();
    } catch (const std::exception& e) {
        std::cerr << "处理获取实时状态请求异常: " << e.what() << std::endl;
        return "";
    }
}

std::string handleNavigationTaskRequestXml(const std::string& request_data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(request_data.begin(), request_data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return "";

        rapidxml::xml_node<>* time_node = root->first_node("Time");
        std::string timestamp = time_node ? time_node->value() : getCurrentTimestamp();

        // 获取第一个导航点的 Value
        int value = 0;
        for (rapidxml::xml_node<>* items_node = root->first_node("Items");
             items_node;
             items_node = items_node->next_sibling("Items")) {

            rapidxml::xml_node<>* value_node = items_node->first_node("Value");
            if (value_node) {
                value = std::stoi(value_node->value());
                break;
            }
        }

        // 随机生成错误码
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> error_dis(0, 10);
        int error_code = error_dis(gen) < 8 ? 0 : 1; // 80% 成功率

        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1003</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << "  <Value>" << value << "</Value>\n";
        ss << "  <ErrorCode>" << error_code << "</ErrorCode>\n";
        ss << "  <ErrorStatus>0</ErrorStatus>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";

        return ss.str();
    } catch (const std::exception& e) {
        std::cerr << "处理导航任务请求异常: " << e.what() << std::endl;
        return "";
    }
}

std::string handleQueryStatusRequestXml(const std::string& request_data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(request_data.begin(), request_data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return "";

        rapidxml::xml_node<>* time_node = root->first_node("Time");
        std::string timestamp = time_node ? time_node->value() : getCurrentTimestamp();

        // 随机生成状态
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> status_dis(0, 2);
        int status = status_dis(gen);
        if (status == 2) status = -1; // 将2映射为-1

        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1007</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << "  <Value>0</Value>\n";
        ss << "  <Status>" << status << "</Status>\n";
        ss << "  <ErrorCode>0</ErrorCode>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";

        return ss.str();
    } catch (const std::exception& e) {
        std::cerr << "处理查询状态请求异常: " << e.what() << std::endl;
        return "";
    }
}

std::string handleCancelTaskRequestXml(const std::string& request_data) {
    try {
        rapidxml::xml_document<> doc;
        std::vector<char> buffer(request_data.begin(), request_data.end());
        buffer.push_back('\0');
        doc.parse<rapidxml::parse_non_destructive>(&buffer[0]);

        rapidxml::xml_node<>* root = doc.first_node("PatrolDevice");
        if (!root) return "";

        rapidxml::xml_node<>* time_node = root->first_node("Time");
        std::string timestamp = time_node ? time_node->value() : getCurrentTimestamp();

        // 随机生成错误码
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> error_dis(0, 10);
        int error_code = error_dis(gen) < 9 ? 0 : 1; // 90% 成功率

        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<PatrolDevice>\n";
        ss << "<Type>1004</Type>\n";
        ss << "<Command>1</Command>\n";
        ss << "<Time>" << timestamp << "</Time>\n";
        ss << "<Items>\n";
        ss << " <ErrorCode>" << error_code << "</ErrorCode>\n";
        ss << "</Items>\n";
        ss << "</PatrolDevice>";

        return ss.str();
    } catch (const std::exception& e) {
        std::cerr << "处理取消任务请求异常: " << e.what() << std::endl;
        return "";
    }
}

// 添加获取当前时间戳的辅助函数
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
