cmake_minimum_required(VERSION 3.5)
project(RCore
    VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build the 'Criterion' Test Suite and enable testing" OFF)

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)

add_subdirectory(third_party)
if(BUILD_TESTS)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/Criterion/include)
endif()

add_subdirectory(librbp)
add_subdirectory(librsp)
add_subdirectory(librcp)
add_subdirectory(librcore)
