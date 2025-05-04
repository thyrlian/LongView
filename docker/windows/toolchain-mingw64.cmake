# toolchain-mingw64.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Set the cross compiler prefix
set(CROSS_PREFIX x86_64-w64-mingw32)

# Set compilers
set(CMAKE_C_COMPILER ${CROSS_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${CROSS_PREFIX}-windres)

# Define the sysroot and library/include search paths
set(CMAKE_FIND_ROOT_PATH
    /usr/${CROSS_PREFIX}
    /usr/${CROSS_PREFIX}/sys-root
)

# Control how CMake searches for packages and libraries
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optional: Disable shared libs (build static)
set(BUILD_SHARED_LIBS OFF)