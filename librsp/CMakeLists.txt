set(LIBRSP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_message.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rsp STATIC
	${LIBRSP_SRCS})
else()
    add_library(rsp STATIC
	${LIBRSP_SRCS})
endif()

set_property(TARGET rsp
    PROPERTY C_STANDARD 11)

target_include_directories(rsp${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rsp${LIBRARY_SUFFIX} rcp${LIBRARY_SUFFIX})

# Endian
if(BIG_ENDIAN)
    target_compile_definitions(rsp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=1)
else()
    target_compile_definitions(rsp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=0)
endif()

# Tests
if(BUILD_TESTS)
    add_executable(rsp-test-stream-message ${CMAKE_CURRENT_SOURCE_DIR}/test/test-stream-message.c)
    target_link_libraries(rsp-test-stream-message Criterion rsp rbp)
    add_test(
	NAME rsp-test-stream-message
	COMMAND rsp-test-stream-message)
endif()
