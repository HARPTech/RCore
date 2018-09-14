if(BUILD_TESTS)
    # Try to find Criterion first, if it is not
    # found, then build it from the submodule.
    find_package(Criterion)

    if(NOT CRITERION_FOUND)
	add_subdirectory(Criterion)
	add_library(Criterion IMPORTED)
	target_include_directories(Criterion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Criterion/include)
	target_link_libraries(Criterion criterion)
    endif()
endif()

# Klib
add_library(klib INTERFACE)
target_include_directories(klib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/)

# SDS
add_library(sds STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sds/sds.c)
target_include_directories(sds SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/sds)
