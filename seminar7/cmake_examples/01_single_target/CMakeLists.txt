cmake_minimum_required(VERSION 3.22)
project(spread_demo LANGUAGES CXX)

add_executable(spread_demo main.cpp)
target_compile_features(spread_demo PRIVATE cxx_std_17)
set_target_properties(spread_demo PROPERTIES CXX_EXTENSIONS OFF)
