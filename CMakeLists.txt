cmake_minimum_required(VERSION 3.20)
project(Xqqyt_SignaleServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_PATH
    MainServer/main.cpp
)

set(INCLUDE_PATH
)

find_package(nlohmann_json REQUIRED)

include_directories(/usr/include/x86_64-linux-gnu)

add_executable(${PROJECT_NAME} ${SRC_PATH} ${INCLUDE_PATH})


target_link_libraries(${PROJECT_NAME}
    boost_system
    pthread
    nlohmann_json::nlohmann_json
)