cmake_minimum_required(VERSION 3.14)
project(RingBuffer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_library(ring_buffer INTERFACE)
target_include_directories(ring_buffer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ring_buffer_demo src/main.cpp)
target_link_libraries(ring_buffer_demo PRIVATE ring_buffer)

add_executable(ring_buffer_tests tests/test_ring_buffer.cpp)
target_link_libraries(ring_buffer_tests PRIVATE ring_buffer Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(ring_buffer_tests)
