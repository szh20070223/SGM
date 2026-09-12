cmake_minimum_required(VERSION 3.16)

project(spring3 LANGUAGES CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(t STATIC
    src/global_data.cpp
    src/global_information.cpp
)

target_include_directories(t PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(spring3
    src/main.cpp
)

target_link_libraries(spring3 PRIVATE
    t
    
)