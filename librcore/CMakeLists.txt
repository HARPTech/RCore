set(LIBRCORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rcore.c
    )

add_library(rcore STATIC
    ${LIBRCORE_SRCS})

target_include_directories(rcore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rcore rbp)
