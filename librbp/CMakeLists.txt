set(LIBRBP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crc.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rbp STATIC
	${LIBRBP_SRCS})
else()
    add_library(rbp SHARED
	${LIBRBP_SRCS})
    set_property(TARGET rbp PROPERTY POSITION_INDEPENDENT_CODE ON)
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

# Install only when compiled with RBase
if(INSTALL_RBASE)
    include(GNUInstallDirs)

    install(TARGETS rbp
	    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static)
endif()
