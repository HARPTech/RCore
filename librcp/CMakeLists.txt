set(LIBRCP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types.c
    )

add_library(rcp STATIC
    ${LIBRCP_SRCS})

target_include_directories(rcp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
