set(LIBRBP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block.c
    )

add_library(rbp STATIC
    ${LIBRBP_SRCS})

target_include_directories(rbp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rbp rsp)

# Tests
add_executable(rbp-test-block ${CMAKE_CURRENT_SOURCE_DIR}/test/test-block.c)
target_link_libraries(rbp-test-block criterion rbp)
