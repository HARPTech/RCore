# This file finds all test cases from globally defined test
# shared libraries and links them with the test runner.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

add_executable(test-runner ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(test-runner catch)

message(STATUS "Building test-runner is enabled. Looking for tests to include.")

get_property(TEST_TARGETS GLOBAL PROPERTY TEST_SHARED_LIBRARIES)

# Link all libraries.
foreach(target ${TEST_TARGETS})
    message(STATUS "Found test library " ${target} ", adding it to test runner.")
    target_link_libraries(test-runner ${target})
endforeach()
