cmake_minimum_required(VERSION 3.12)
project(AlgoBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SWIG REQUIRED)
find_package(Python3 COMPONENTS Development Interpreter REQUIRED)

include(${SWIG_USE_FILE})
include_directories(${Python3_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(hft_cpp hft_cpp/order_book.cpp hft_cpp/stock_exchange.cpp)
add_executable(hft_client src/main.cpp)

target_link_libraries(hft_client hft_cpp)
