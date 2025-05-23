cmake_minimum_required(VERSION 3.16)
project(LongView VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt modules you use
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Find yaml-cpp using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories(${YAML_CPP_LIBRARY_DIRS})

set(PROJECT_SOURCES
    main.cpp
    windowutils.h
    windowutils.cpp
    appintegration.h
    appintegration.cpp
    config/config_manager.h
    config/config_manager.cpp
    config/config.h
)

# macOS icon
set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/macOS/icon.icns")

# Windows: Add .rc file for application icon
if(WIN32)
    list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/assets/Windows/windows_app_icon.rc")
endif()

# Create the executable
qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${ICON_PATH}
)

# Link to Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${YAML_CPP_LIBRARIES}
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# macOS icon packaging
set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set_source_files_properties(${ICON_PATH} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

# Install directives
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)