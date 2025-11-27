cmake_minimum_required(VERSION 3.20)
project("Software Engineering and C++ for Quantitative Analysis and Algorithmic Trading - Backtester" LANGUAGES CXX)

option(BUILD_EXAMPLES "Build example CLI apps" ON)
option(BUILD_DOCS "Build API docs with Doxygen" ON) # requires Doxygen installation (e.g. `brew install doxygen graphviz` for macOS)
option(BUILD_PY_SWIG "Build Python module" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_LOCAL_FLAGS -O3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libbacktester
    src/libbacktester/orderbook.cpp

    src/libbacktester/strategies/strategy_factory.cpp
    src/libbacktester/strategies/twap_strategy.cpp

    src/libbacktester/execution_simulator.cpp
    src/libbacktester/metrics_writer.cpp
    src/libbacktester/portfolio.cpp

    src/libbacktester/simulation_core.cpp

    src/libbacktester/bindings.cpp
)
target_include_directories(libbacktester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(libbacktester PRIVATE -Wall -Wextra -Wpedantic ${CMAKE_CXX_LOCAL_FLAGS})

if (BUILD_EXAMPLES)
    add_executable(demo_order_book
        "${CMAKE_CURRENT_SOURCE_DIR}/src/app/demo_order_book.cpp"
    )
    target_link_libraries(demo_order_book PRIVATE libbacktester)
    target_compile_options(demo_order_book PRIVATE ${CMAKE_CXX_LOCAL_FLAGS})

    add_executable(demo_portfolio_execution
        "${CMAKE_CURRENT_SOURCE_DIR}/src/app/demo_portfolio_execution.cpp"
    )
    target_link_libraries(demo_portfolio_execution PRIVATE libbacktester)
endif()


if (BUILD_DOCS)
    find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()

if (BUILD_PY_SWIG)
    find_package(SWIG REQUIRED)
    include(UseSWIG)
    find_package(PythonLibs)

    include_directories(${PYTHON_INCLUDE_DIRS})

    set_source_files_properties(python/pylibbacktester.i PROPERTIES
        CPLUSPLUS ON
        SWIG_FLAGS "-directors"
    )

    swig_add_library(pylibbacktester
        TYPE MODULE
        LANGUAGE python
        SOURCES python/pylibbacktester.i
    )

    target_link_libraries(pylibbacktester PRIVATE libbacktester ${PYTHON_LIBRARIES})
    target_include_directories(pylibbacktester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_property(
        TARGET pylibbacktester
        PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
    )

    set_target_properties(pylibbacktester PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
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

add_executable(test_orderbook
    tests/test_orderbook.cpp
)
target_link_libraries(test_orderbook PRIVATE libbacktester gtest_main)
add_test(NAME test_orderbook COMMAND test_orderbook)

add_executable(test_twap
  tests/test_twap.cpp
)
target_link_libraries(test_twap PRIVATE libbacktester GTest::gtest_main)
add_test(NAME test_twap COMMAND test_twap)

add_executable(test_portfolio_execution
  tests/test_execution_simulator.cpp
  tests/test_portfolio.cpp
)
target_link_libraries(test_portfolio_execution PRIVATE libbacktester GTest::gtest_main)
add_test(NAME test_portfolio_execution COMMAND test_portfolio_execution)

add_executable(test_metrics
  tests/test_metrics_writer.cpp
)
target_link_libraries(test_metrics PRIVATE libbacktester GTest::gtest_main)
add_test(NAME test_metrics COMMAND test_metrics)


add_executable(test_simulation_core
  tests/test_simulation_core.cpp
)
target_link_libraries(test_simulation_core PRIVATE libbacktester GTest::gtest_main)
add_test(NAME simulation_core_tests COMMAND test_simulation_core)
