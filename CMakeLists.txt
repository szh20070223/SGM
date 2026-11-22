cmake_minimum_required(VERSION 3.16)

project(spring3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(nlohmann_json INTERFACE)

target_include_directories(nlohmann_json INTERFACE
    ${PROJECT_SOURCE_DIR}/third_party
)

add_library(t STATIC
    src/GradeSystem.cpp
    src/persistence.cpp
    src/menus_stub.cpp
)

target_include_directories(t PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(t PRIVATE
    nlohmann_json
)

add_executable(spring3
    src/main.cpp
)

target_link_libraries(spring3 PRIVATE
    t
)