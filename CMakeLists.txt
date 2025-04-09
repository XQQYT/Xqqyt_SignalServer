cmake_minimum_required(VERSION 3.20)
project(Xqqyt_SignaleServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源码路径（确保包含所有.cpp文件）
set(SRC_PATH
    MainServer/main.cpp
    MainServer/Server.cpp
    MainServer/MessageParser.cpp  # 添加MessageParser的实现文件
    MainServer/Strategy/Strategy.cpp      # 添加Strategy的实现文件
)

# 设置头文件路径
set(INCLUDE_PATH
    MainServer/Strategy
    MainServer
)

# 查找依赖
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

# 包含目录
include_directories(
    ${INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
    /usr/include/x86_64-linux-gnu
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SRC_PATH})

# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Boost::boost
    Boost::system
    pthread
    nlohmann_json::nlohmann_json
)