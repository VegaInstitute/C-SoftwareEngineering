cmake_minimum_required(VERSION 3.14)
project(ch_example)

set(CMAKE_CXX_STANDARD 17)

add_executable(main main.cpp)

include(FetchContent)

FetchContent_Declare(
  ch
  GIT_REPOSITORY https://github.com/ClickHouse/clickhouse-cpp.git
  GIT_TAG v2.5.1
)
FetchContent_MakeAvailable(ch)

target_link_libraries(main clickhouse-cpp-lib)
