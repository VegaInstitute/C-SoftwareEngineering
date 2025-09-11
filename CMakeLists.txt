cmake_minimum_required(VERSION 3.20)
project(CPPForQuantFinance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libcustommath src/libcustommath/math.cpp)
target_include_directories(libcustommath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(sem1_calc src/app/sem1_calculator.cpp)
target_link_libraries(sem1_calc PRIVATE libcustommath)

include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(test_math tests/test_math.cpp)
target_link_libraries(test_math PRIVATE libcustommath gtest_main)
add_test(NAME test_math COMMAND test_math)
