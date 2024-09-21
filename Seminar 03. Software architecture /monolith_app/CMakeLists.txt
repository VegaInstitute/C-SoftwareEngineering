cmake_minimum_required(VERSION 3.15)
project(monolith_options_pricing)

# Set C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(monolith_options_pricing
  src/options_pricing.cpp
  src/black_scholes.cpp
)

# Add include directories
include_directories(include)
