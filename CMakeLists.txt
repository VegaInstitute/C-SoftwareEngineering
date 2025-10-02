cmake_minimum_required(VERSION 3.20)
project(CPPForQuantFinance LANGUAGES CXX)

option(BUILD_EXAMPLES "Build example CLI apps" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libbacktester src/libbacktester/orderbook.cpp)
target_include_directories(libbacktester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (BUILD_EXAMPLES)
  add_executable(demo_order_book
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app/demo_order_book.cpp"
  )
  target_link_libraries(demo_order_book PRIVATE libbacktester)
endif()

include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(test_orderbook tests/test_orderbook.cpp)
target_link_libraries(test_orderbook PRIVATE libbacktester gtest_main)
add_test(NAME test_orderbook COMMAND test_orderbook)
