cmake_minimum_required(VERSION 3.20)
project(research_platform_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
if(MINGW)
  add_link_options(-static)
endif()

# Domain: Bar, IBarSource (port), count_gaps. Knows nothing about adapters.
add_library(rp_domain STATIC domain/quality.cpp)
target_include_directories(rp_domain PUBLIC domain)
# target_link_libraries(rp_domain PUBLIC rp_adapters)  # DEMO-2

# Adapters: IssSource, BrokerCsvSource, FailoverSource. Depend on the domain.
add_library(rp_adapters STATIC adapters/iss_source.cpp adapters/broker_csv_source.cpp)
target_include_directories(rp_adapters PUBLIC adapters)
target_link_libraries(rp_adapters PUBLIC rp_domain)

# Composition root.
add_executable(rp_cli app/main.cpp)
target_link_libraries(rp_cli PRIVATE rp_adapters)
target_compile_definitions(rp_cli PRIVATE "RP_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")

enable_testing()
add_executable(failover_test tests/failover_test.cpp)
target_link_libraries(failover_test PRIVATE rp_adapters)
add_test(NAME failover_test COMMAND failover_test)

# Fitness function for ADR-0003: the domain must not depend on adapters.
get_target_property(_deps rp_domain LINK_LIBRARIES)
if(_deps MATCHES "rp_adapters")
  message(FATAL_ERROR "ADR-0003 violated: rp_domain must not depend on rp_adapters (see docs/adr/0003-domain-independent-of-adapters.md)")
endif()
