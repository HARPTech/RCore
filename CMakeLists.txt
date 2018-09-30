cmake_minimum_required(VERSION 3.5)
project(RCore
    VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build the 'Criterion' Test Suite and enable testing" ON)
option(USE_AVR_LIBRARIES "Use AVR library calls instead of standard ones for Arduino compat" OFF)

define_property(GLOBAL PROPERTY TEST_SHARED_LIBRARIES
    BRIEF_DOCS "All shared libraries containing tests."
    FULL_DOCS "All shared libraries containing tests, will be linked by the test runner.")
set_property(GLOBAL PROPERTY TEST_SHARED_LIBRARIES "")

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)

add_subdirectory(third_party)
if(BUILD_TESTS)
    enable_testing()
endif()

set(LIBRARY_SUFFIX "")

if(USE_AVR_LIBRARIES)
    set(LIBRARY_SUFFIX -${AVR_MCU})
endif()

add_subdirectory(librbp)
add_subdirectory(librsp)
add_subdirectory(librcp)
add_subdirectory(librcore)

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    if(BUILD_TESTS)
	add_subdirectory(test)
    endif()
endif()
