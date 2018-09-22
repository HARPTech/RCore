set(LIBRBP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.c
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

target_link_libraries(rbp${LIBRARY_SUFFIX} rsp${LIBRARY_SUFFIX} klib${LIBRARY_SUFFIX})

# Tests
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
