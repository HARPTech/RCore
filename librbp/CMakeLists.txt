set(LIBRBP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block.c
    )

add_library(rbp STATIC
    ${LIBRBP_SRCS})

set_property(TARGET rbp PROPERTY C_STANDARD 11)

target_include_directories(rbp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rbp rsp)

# Tests
if(BUILD_TESTS)
    add_executable(rbp-test-block ${CMAKE_CURRENT_SOURCE_DIR}/test/test-block.c)
    target_link_libraries(rbp-test-block Criterion rbp)
endif()
