set(LIBRSP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flags.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/litecomm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data.c
    )

if(USE_AVR_LIBRARIES)
    add_avr_library(rsp STATIC
	${LIBRSP_SRCS})
else()
    add_library(rsp STATIC
	${LIBRSP_SRCS})
    set_property(TARGET rsp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

set_property(TARGET rsp
    PROPERTY C_STANDARD 11)

target_include_directories(rsp${LIBRARY_SUFFIX}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(rsp${LIBRARY_SUFFIX} rcp${LIBRARY_SUFFIX})

# Endian
if(BIG_ENDIAN)
    target_compile_definitions(rsp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=1)
else()
    target_compile_definitions(rsp${LIBRARY_SUFFIX} PUBLIC -DBIG_ENDIAN=0)
endif()

# Tests
if(BUILD_TESTS)
    add_subdirectory(test)
endif()

# Install only when compiled with RBase
if(INSTALL_RBASE)
    include(GNUInstallDirs)

    install(TARGETS rsp
	    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static)
endif()
