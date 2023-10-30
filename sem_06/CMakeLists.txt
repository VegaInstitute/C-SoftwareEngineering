cmake_minimum_required(VERSION 3.14)
project(io_aware_matmul)

set(CMAKE_CXX_STANDARD 17)

add_library(matmul matmul.cpp)

add_executable(main main.cpp)
target_link_libraries(main matmul)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929 # v1.11.0
)
FetchContent_MakeAvailable(googletest)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_LIBPFM ON)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG 344117638c8ff7e239044fd0fa7085839fc03021 # v1.8.3
)
FetchContent_MakeAvailable(benchmark)
add_executable(bench benchmark.cpp)
target_link_libraries(bench matmul benchmark::benchmark)

include(cmake/FetchBoostContent.cmake)
fetchboostcontent_declare(
  boost_multi_array GIT_REPOSITORY https://github.com/boostorg/multi_array
  GIT_TAG 0c5348bef71b890c4bd06eff1ee5ebda69e7b27a # v1.79.0
)
fetchboostcontent_makeavailable(boost_multi_array)

enable_testing()

add_executable(tests tests.cpp)
target_link_libraries(tests matmul gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
