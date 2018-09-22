if(BUILD_TESTS)
    add_subdirectory(catch)
endif()

# Klib
add_library(klib INTERFACE)
target_include_directories(klib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/)
