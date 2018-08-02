set(LIBRCORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rcore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/subscription-map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashtable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transmit_buffer.c
    )

add_library(rcore STATIC
    ${LIBRCORE_SRCS})

set_property(TARGET rcore PROPERTY C_STANDARD 11)

target_include_directories(rcore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rcore rbp)
target_link_libraries(rcore klib)

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
