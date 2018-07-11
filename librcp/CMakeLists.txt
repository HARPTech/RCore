set(LIBRCP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types.c
    )

add_library(rcp STATIC
    ${LIBRCP_SRCS})

set_property(TARGET rcp PROPERTY C_STANDARD 11)

target_include_directories(rcp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Endian
if(BIG_ENDIAN)
    target_compile_definitions(rcp -DBIG_ENDIAN)
endif()

# Tests
if(BUILD_TESTS)
    add_executable(rcp-test-message ${CMAKE_CURRENT_SOURCE_DIR}/test/test-message.c)
    target_link_libraries(rcp-test-message Criterion rcp rsp rbp)
    add_test(
	NAME rcp-test-message
	COMMAND rcp-test-message)
endif()
