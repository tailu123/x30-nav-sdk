# server 示例目录的 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 模拟服务器
add_executable(mock_server mock_server.cpp)
target_link_libraries(mock_server PRIVATE ${Boost_LIBRARIES} nlohmann_json::nlohmann_json Threads::Threads)

# 安装示例
install(TARGETS mock_server
    RUNTIME DESTINATION bin/examples/server
)
