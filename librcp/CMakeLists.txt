set(LIBRCP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rcp STATIC
	${LIBRCP_SRCS})
else()
    add_library(rcp STATIC
	${LIBRCP_SRCS})
endif()

set_property(TARGET rcp PROPERTY C_STANDARD 11)
set_property(TARGET rcp PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(rcp${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Endian
if(BIG_ENDIAN)
    target_compile_definitions(rcp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=1)
else()
    target_compile_definitions(rcp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=0)
endif()

# Tests
if(BUILD_TESTS)
    add_executable(rcp-test-message ${CMAKE_CURRENT_SOURCE_DIR}/test/test-message.c)
    target_link_libraries(rcp-test-message Criterion rcp rsp rbp)
    add_test(
	NAME rcp-test-message
	COMMAND rcp-test-message)
endif()
