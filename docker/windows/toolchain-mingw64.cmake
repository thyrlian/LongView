# toolchain-mingw64.cmake

# Specify the target system and processor
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Define the cross-compiler prefix
set(CROSS_PREFIX x86_64-w64-mingw32)

# Set compilers for C, C++, and resource files
set(CMAKE_C_COMPILER ${CROSS_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${CROSS_PREFIX}-windres)

# Define root paths for locating headers and libraries
set(CMAKE_FIND_ROOT_PATH
    /usr/${CROSS_PREFIX}
    /usr/${CROSS_PREFIX}/sys-root
    /opt/qt-win64               # Qt for Windows installation path
)

# Help CMake find Qt CMake config files
set(CMAKE_PREFIX_PATH "/opt/qt-win64;/opt/qt-win64/lib/cmake")

# Control how CMake searches for programs, libraries, and headers
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optional: enable for static builds (comment out if not needed)
set(BUILD_SHARED_LIBS OFF)
