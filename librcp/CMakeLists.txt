set(LIBRCP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rcp STATIC
	${LIBRCP_SRCS})
else()
    add_library(rcp SHARED
	${LIBRCP_SRCS})
    set_property(TARGET rcp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

set_property(TARGET rcp PROPERTY C_STANDARD 11)

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
    add_subdirectory(test)
endif()

# Install only when compiled with RBase
if(INSTALL_RBASE)
    include(GNUInstallDirs)

    install(TARGETS rcp
	    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static)
endif()
