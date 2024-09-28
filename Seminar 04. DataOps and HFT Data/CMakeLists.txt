cmake_minimum_required(VERSION 3.12)
project(HFTDataLoader)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SWIG REQUIRED)
find_package(Python3 COMPONENTS Development Interpreter REQUIRED)

include(${SWIG_USE_FILE})
include_directories(${Python3_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(hft_cpp hft_cpp/hft_data_loader.cpp)
add_executable(hft_client src/main.cpp)

target_link_libraries(hft_client hft_cpp)

set_property(SOURCE python/hft_python.i PROPERTY CPLUSPLUS ON)
swig_add_library(hft_python LANGUAGE python SOURCES python/hft_python.i hft_cpp/hft_data_loader.cpp)
swig_link_libraries(hft_python hft_cpp ${Python3_LIBRARIES})
