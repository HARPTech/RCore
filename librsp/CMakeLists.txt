set(LIBRSP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_message.c
    )

add_library(rsp STATIC
    ${LIBRSP_SRCS})

target_include_directories(rsp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rsp rcp)
