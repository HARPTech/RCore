if(BUILD_TESTS)
    add_subdirectory(catch)
endif()

# Klib
if(USE_AVR_LIBRARIES)
    add_avr_library(klib ${CMAKE_CURRENT_SOURCE_DIR}/klib-dummy.c)
else()
    add_library(klib INTERFACE)
    target_include_directories(klib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()
