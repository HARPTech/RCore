set(LIBRCORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rcore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/subscription-map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashtable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transmit_buffer.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rcore STATIC
	${LIBRCORE_SRCS})
else()
    add_library(rcore STATIC
	${LIBRCORE_SRCS})
endif()

set_property(TARGET rcore PROPERTY POSITION_INDEPENDENT_CODE ON)

set_property(TARGET rcore PROPERTY C_STANDARD 11)

target_include_directories(rcore${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rcore${LIBRARY_SUFFIX} rbp${LIBRARY_SUFFIX})
target_link_libraries(rcore${LIBRARY_SUFFIX} klib)

# Tests
if(BUILD_TESTS)
    add_executable(rcore-test-rcomm
	${CMAKE_CURRENT_SOURCE_DIR}/test/test-rcomm.c
	${CMAKE_CURRENT_SOURCE_DIR}/test/test-subscription-map.c)
    target_link_libraries(rcore-test-rcomm Criterion rcore)
    add_test(
	NAME rcore-test-rcomm
	COMMAND rcore-test-rcomm)
endif()
