cmake_minimum_required(VERSION 3.16)
project(financial_data_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

foreach(name IN ITEMS parse_bench book_bench)
    add_executable(${name} ${name}.cpp)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

enable_testing()
add_test(NAME parser_contract COMMAND parse_bench --check)
add_test(NAME book_equivalence COMMAND book_bench --check --quick)
