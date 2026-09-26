cmake_minimum_required(VERSION 3.10)

project(http_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -g)


add_executable(http_server 
            src/main.cpp
            src/net/EventLoop.cpp
            src/net/Acceptor.cpp)

target_include_directories(http_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src)