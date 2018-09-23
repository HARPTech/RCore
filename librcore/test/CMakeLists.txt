# Variables defining the test.
set(TEST_NAME rcore-test)
set(TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/test-rcomm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-sequence-stack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-ack-stack.cpp
    )
set(TEST_LIBS rcore)

# Here begins the setup of the test.
add_library(${TEST_NAME} SHARED ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS} catch)
set_property(GLOBAL APPEND PROPERTY TEST_SHARED_LIBRARIES "${TEST_NAME}")
