cmake_minimum_required(VERSION 3.20)
project(micro_router LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(micro_router INTERFACE)
add_library(micro_router::micro_router ALIAS micro_router)

target_include_directories(micro_router INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(micro_router INTERFACE /W4 /permissive-)
else()
  target_compile_options(micro_router INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(micro_router_basic_test tests/test_basic.cpp)
target_link_libraries(micro_router_basic_test PRIVATE micro_router::micro_router)
add_test(NAME micro_router.basic COMMAND micro_router_basic_test)
