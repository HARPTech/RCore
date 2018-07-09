cmake_minimum_required(VERSION 3.5)
project(RCore
    VERSION 0.1.0)

add_subdirectory(third_party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/Criterion/include)

add_subdirectory(librbp)
add_subdirectory(librsp)
add_subdirectory(librcp)
add_subdirectory(librcore)
