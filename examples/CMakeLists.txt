# 示例程序的CMakeLists.txt

# 查找线程库
find_package(Threads REQUIRED)

# 基本示例
add_executable(basic_example basic_example.cpp)
target_link_libraries(basic_example PRIVATE ${PROJECT_NAME} Threads::Threads)

# 新的基本示例
add_executable(basic_example_new basic_example_new.cpp)
target_link_libraries(basic_example_new PRIVATE ${PROJECT_NAME} Threads::Threads)

# 模拟服务器
add_executable(mock_server mock_server.cpp)
target_link_libraries(mock_server PRIVATE ${Boost_LIBRARIES} nlohmann_json::nlohmann_json Threads::Threads)

# 安装示例
install(TARGETS basic_example basic_example_new mock_server
    RUNTIME DESTINATION bin/examples
)

# 复制示例源代码到安装目录
install(FILES
    basic_example.cpp
    basic_example_new.cpp
    mock_server.cpp
    DESTINATION share/${PROJECT_NAME}/examples
)
