# 示例程序的CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(x30_nav_sdk_examples)

# 查找线程库
find_package(Threads REQUIRED)

# 新的基本示例
add_executable(basic_example_new basic_example_new.cpp)
target_link_libraries(basic_example_new PRIVATE x30_nav_sdk Threads::Threads)

# 模拟服务器
add_executable(mock_server mock_server.cpp)
target_link_libraries(mock_server PRIVATE ${Boost_LIBRARIES} nlohmann_json::nlohmann_json Threads::Threads)

# 安装示例
install(TARGETS basic_example_new mock_server
    RUNTIME DESTINATION bin/examples
)

# 复制示例源代码到安装目录
install(FILES
    basic_example_new.cpp
    mock_server.cpp
    DESTINATION share/x30_nav_sdk/examples
)
