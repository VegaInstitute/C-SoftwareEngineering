cmake_minimum_required(VERSION 3.12)
project(PSQLConnector)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PostgreSQL REQUIRED)

add_executable(main src/main.c)
target_link_libraries(main PRIVATE PostgreSQL::PostgreSQL)
