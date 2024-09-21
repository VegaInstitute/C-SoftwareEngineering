cmake_minimum_required(VERSION 3.15)
project(modular_options_pricing)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(modular_options_pricing
  src/options_pricing.cpp
  src/calculator_black_scholes.cpp
  src/ui.cpp
  src/database.cpp
)

include_directories(include)
