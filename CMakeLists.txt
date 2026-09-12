cmake_minimum_required(VERSION 3.15)
project(spring3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动收集当前目录下所有 .cpp
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(spring3 ${SOURCES})

target_include_directories(spring3 PRIVATE include)