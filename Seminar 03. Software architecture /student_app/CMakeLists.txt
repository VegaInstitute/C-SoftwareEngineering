cmake_minimum_required(VERSION 3.15)
project(options_pricing)

# Set C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(options_pricing
  main.cpp
  black_scholes.cpp
)

# Add include directories
include_directories(include)
