set(LIBRCORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rcore.c
    )

add_library(rcore STATIC
    ${LIBRCORE_SRCS})

set_property(TARGET rcore PROPERTY C_STANDARD 11)

target_include_directories(rcore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rcore rbp)

# Tests
if(BUILD_TESTS)
    add_executable(rcore-test-rcomm ${CMAKE_CURRENT_SOURCE_DIR}/test/test-rcomm.c)
    target_link_libraries(rcore-test-rcomm Criterion rcore)
endif()
