cmake_minimum_required(VERSION 3.16)
project(LongView VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt modules you use
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_SOURCES
    main.cpp
    windowutils.h
    windowutils.cpp
    appintegration.h
    appintegration.cpp
)

set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/macOS/icon.icns")

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${ICON_PATH}
)

# Correctly link to Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Set properties for macOS and Windows
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Set the macOS application icon
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