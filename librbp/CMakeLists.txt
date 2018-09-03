set(LIBRBP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rbp STATIC
	${LIBRBP_SRCS})
else()
    add_library(rbp STATIC
	${LIBRBP_SRCS})
endif()

set_property(TARGET rbp PROPERTY C_STANDARD 11)

target_include_directories(rbp${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rbp${LIBRARY_SUFFIX} rsp${LIBRARY_SUFFIX})

# Tests
if(BUILD_TESTS)
    add_executable(rbp-test-block ${CMAKE_CURRENT_SOURCE_DIR}/test/test-block.c)
    target_link_libraries(rbp-test-block Criterion rbp)
    add_test(
	NAME rbp-test-block
	COMMAND rbp-test-block)
endif()
