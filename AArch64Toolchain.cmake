set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm64)


set(BUILDER /opt/verner-builder-sysroot)
set(UBUNTU1604 /opt/ubuntu-16-04/)

set(REQUIRED_LIBRARY_PATH "/usr/lib/:${UBUNTU1604}/usr/lib:${UBUNTU1604}/usr/lib/x86_64-linux-gnu/")

configure_file(${CMAKE_CURRENT_LIST_DIR}/build-cli.sh.in ${CMAKE_CURRENT_BINARY_DIR}/build-cli.sh)

set(ENV{LD_LIBRARY_PATH} ${REQUIRED_LIBRARY_PATH})

set(CMAKE_SYSROOT ${BUILDER}/)
set(CMAKE_STAGING_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/stage)

set(CMAKE_C_COMPILER ${UBUNTU1604}/usr/bin/aarch64-linux-gnu-gcc-5)
set(CMAKE_CXX_COMPILER ${UBUNTU1604}/usr/bin/aarch64-linux-gnu-g++-5)

set(CMAKE_FIND_ROOT_PATH ${BUILDER}/usr)
list(APPEND CMAKE_PREFIX_PATH ${BUILDER}/usr/lib/aarch64-linux-gnu/)
list(APPEND CMAKE_PREFIX_PATH ${BUILDER}/usr/include/aarch64-linux-gnu/)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L ${BUILDER}/usr/lib/aarch64-linux-gnu/ -ldl -licuuc -licudata -licui18n")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -L ${BUILDER}/usr/lib/aarch64-linux-gnu/ -ldl")

if(NOT TARGET Threads::Threads)
    add_library(Threads::Threads INTERFACE IMPORTED)
endif()
set_property(TARGET Threads::Threads PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
set_property(TARGET Threads::Threads PROPERTY INTERFACE_LINK_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
set(Threads_FOUND 1)

include_directories(${BUILDER}/usr/include/)
include_directories(${BUILDER}/usr/include/aarch64-linux-gnu/)

link_directories(${BUILDER}/usr/lib/aarch64-linux-gnu/)
link_directories(${BUILDER}/usr/lib/gcc/aarch64-linux-gnu/5/)
link_directories(${BUILDER}/usr/lib/gcc/aarch64-linux-gnu/5.3.1/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
