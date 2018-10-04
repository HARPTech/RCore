set(LIBRCORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sequence_stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ack_stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rcomm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/defaults.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/subscription-map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashtable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transmit_buffer.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rcore STATIC
	${LIBRCORE_SRCS})
else()
    add_library(rcore SHARED
	${LIBRCORE_SRCS})
endif()

set_property(TARGET rcore PROPERTY C_STANDARD 11)
set_property(TARGET rcore PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(rcore${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rcore${LIBRARY_SUFFIX} rbp${LIBRARY_SUFFIX})
target_link_libraries(rcore${LIBRARY_SUFFIX} klib${LIBRARY_SUFFIX})

# Tests
if(BUILD_TESTS)
    add_subdirectory(test)
endif()

# Install only when compiled with RBase
if(INSTALL_RBASE)
    include(GNUInstallDirs)

    install(TARGETS rcore
	    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static)
endif()
