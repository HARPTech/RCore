set(LIBRSP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_message.c
    )

add_library(rsp STATIC
    ${LIBRSP_SRCS})

set_property(TARGET rsp
    PROPERTY C_STANDARD 11)

target_include_directories(rsp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rsp rcp)

# Endian
if(BIG_ENDIAN)
    target_compile_definitions(rsp PUBLIC -DBIG_ENDIAN=1)
else()
    target_compile_definitions(rsp PUBLIC -DBIG_ENDIAN=0)
endif()

# Tests
if(BUILD_TESTS)
    add_executable(rsp-test-stream-message ${CMAKE_CURRENT_SOURCE_DIR}/test/test-stream-message.c)
    target_link_libraries(rsp-test-stream-message Criterion rsp rbp)
    add_test(
	NAME rsp-test-stream-message
	COMMAND rsp-test-stream-message)
endif()
