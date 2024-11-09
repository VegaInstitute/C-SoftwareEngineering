cmake_minimum_required(VERSION 3.12)
project(PricingServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(JsonCpp REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(server src/main.cpp)
target_link_libraries(server JsonCpp::JsonCpp)
