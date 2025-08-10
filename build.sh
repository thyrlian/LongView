#!/bin/bash
# Build script for LongView using Docker for cross-platform compilation and packaging.

set -e # Exit immediately if a command exits with a non-zero status.

# --- Configuration ---
DOCKER_IMAGE_LINUX="qt6-dev-ubuntu" # Name of your Qt development Docker image for Linux
BUILD_DIR="build"
DIST_DIR="dist"
APP_NAME="LongView"
ORG_DOMAIN="com.basgeekball"
APP_ID="${ORG_DOMAIN}.${APP_NAME}"
APP_VERSION="0.1" # Example version

# --- Helper Functions ---
print_usage() {
    echo "Usage: $0 [options] <platform>"
    echo "  platform: linux, windows, macos"
    echo "Options:"
    echo "  --target=<platform>  Alternative way to specify platform"
    echo "  --clean              Clean build directories before building"
    exit 1
}

# Clean previous build artifacts for the specified platform
clean_platform() {
    local platform=$1
    echo "--- Cleaning build and dist directories for ${platform} ---"
    rm -rf "${BUILD_DIR}/${platform}"
    mkdir -p "${BUILD_DIR}/${platform}"
    mkdir -p "${DIST_DIR}/${platform}" # Ensure dist directory exists
}

# --- Argument Parsing ---
PLATFORM=""
CLEAN_BUILD=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --target=*)
            PLATFORM="${1#*=}"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        linux|windows|macos)
            PLATFORM="$1"
            shift
            ;;
        -h|--help)
            print_usage
            ;;
        *)
            echo "Error: Unknown parameter '$1'"
            print_usage
            ;;
    esac
done

# Check if a platform was specified
if [ -z "$PLATFORM" ]; then
    echo "Error: No platform specified"
    print_usage
fi

CMAKE_PLATFORM_ARGS="" # Platform specific CMake arguments (toolchains, etc.)

# --- Platform Specific Setup ---
case "$PLATFORM" in
    linux)
        # Apply clean if requested
        if [ "$CLEAN_BUILD" = true ]; then
            clean_platform linux
        fi
        
        # Create host directories that will be mapped to Docker
        HOST_DIST_DIR="${PWD}/${DIST_DIR}/linux"
        HOST_BUILD_DIR="${PWD}/${BUILD_DIR}/linux"
        
        echo "--- Starting build for ${PLATFORM} in Docker ---"
        echo "Host dist directory: ${HOST_DIST_DIR}"
        
        # Run the Linux build in Docker with explicit volume mounts
        docker run --rm \
            -v "${PWD}:/app" \
            -v "${HOST_DIST_DIR}:/output" \
            -w /app \
            ${DOCKER_IMAGE_LINUX} /bin/bash -c "
                set -e
                echo '--- Configuring CMake ---'
                mkdir -p '${BUILD_DIR}/linux'
                cd '${BUILD_DIR}/linux'
                cmake -DCMAKE_BUILD_TYPE=Release /app/src
                
                echo '--- Building application ---'
                make -j\$(nproc)
                
                echo '--- Creating AppDir ---'
                APPDIR=AppDir
                rm -rf \$APPDIR
                mkdir -p \$APPDIR/usr/bin
                mkdir -p \$APPDIR/usr/applications
                mkdir -p \$APPDIR/usr/icons
                
                echo '--- Copying binary and assets ---'
                cp './${APP_NAME}' \$APPDIR/usr/bin/
                cp /app/src/assets/icons/Linux/icon.png \$APPDIR/usr/icons/longview.png
                cp /app/src/assets/desktop/longview.desktop \$APPDIR/usr/applications/longview.desktop
                # Create root .desktop file (required by some AppImage tools)
                cp /app/src/assets/desktop/longview.desktop \$APPDIR/longview.desktop
                
                # Set up icon in root as well
                cp /app/src/assets/icons/Linux/icon.png \$APPDIR/longview.png
                ln -sf longview.png \$APPDIR/.DirIcon
                
                # Copy yaml-cpp library
                mkdir -p \$APPDIR/usr/lib
                find /usr/lib -name "libyaml-cpp.so*" -exec cp {} \$APPDIR/usr/lib/ \; 2>/dev/null || true
                
                # Modify .desktop files to use AppRun
                sed -i 's|Exec=.*|Exec=AppRun|g' \$APPDIR/longview.desktop
                sed -i 's|Exec=.*|Exec=AppRun|g' \$APPDIR/usr/applications/longview.desktop
                
                # Create AppRun script
                cat > \$APPDIR/AppRun << 'EOFAPPRUN'
#!/bin/sh
HERE=\$(dirname \$(readlink -f \${0}))
export PATH=\${HERE}/usr/bin:\${PATH}
export LD_LIBRARY_PATH=\${HERE}/usr/lib:\${LD_LIBRARY_PATH}
export XDG_DATA_DIRS=\${HERE}/usr:\${XDG_DATA_DIRS}
\${HERE}/usr/bin/${APP_NAME} \"\$@\"
EOFAPPRUN
                chmod +x \$APPDIR/AppRun
                
                echo '--- Creating AppImage ---'
                # Check if appimagetool is installed
                if ! command -v appimagetool &> /dev/null; then
                    echo 'Error: appimagetool not found. Please ensure it is installed in Docker image.'
                    exit 1
                fi
                
                OUTPUT_APPIMAGE='/output/${APP_NAME}-${APP_VERSION}-x86_64.AppImage'
                appimagetool -v \$APPDIR \$OUTPUT_APPIMAGE
                
                # Set executable permission
                chmod +x \$OUTPUT_APPIMAGE
                
                echo '--- AppImage created at \$OUTPUT_APPIMAGE ---'
                ls -la /output/
                
                # Copy test executable to output directory
                cp './${APP_NAME}Test' /output/
                echo '--- Test executable copied to output ---'
            "
        
        echo "--- Linux build completed ---"
        echo "AppImage available at: ${DIST_DIR}/linux/${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
        ;;
        
    windows)
        # Apply clean if requested
        if [ "$CLEAN_BUILD" = true ]; then
            clean_platform windows
        fi

        echo "--- Starting native Windows build ---"

        SRC_DIR="${PWD}/src"
        BUILD_SUBDIR="${PWD}/${BUILD_DIR}/windows"
        DIST_SUBDIR="${PWD}/${DIST_DIR}/windows"

        rm -rf "$BUILD_SUBDIR"
        mkdir -p "$BUILD_SUBDIR"
        mkdir -p "$DIST_SUBDIR"

        cd "$BUILD_SUBDIR"

        echo "--- Locating Qt6 installation ---"
        QT_CMAKE_DIR=$(find /c/Qt/6.* -type d -path "*/msvc*/lib" -exec test -d "{}/cmake/Qt6" ';' -print -quit 2>/dev/null | sed 's:/lib$::')

        if [ -z "$QT_CMAKE_DIR" ]; then
            echo "❌ Could not find a suitable Qt6 installation under C:/Qt"
            echo "Please install Qt6 for MSVC (e.g., C:/Qt/6.8.3/msvc2022_64) or set CMAKE_PREFIX_PATH manually."
            exit 1
        fi

        echo "✅ Found Qt installation: $QT_CMAKE_DIR"

        echo "--- Detecting available Visual Studio ---"

        VSWHERE_PATH="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
        if [ ! -f "$VSWHERE_PATH" ]; then
            echo "❌ vswhere.exe not found at $VSWHERE_PATH"
            echo "Please install or ensure Visual Studio Installer exists."
            exit 1
        fi

        VS_GENERATOR=$(
          "$VSWHERE_PATH" -latest -requires Microsoft.Component.MSBuild \
            -property installationVersion | grep -Eo '^[0-9]+' | head -1 | awk '{
              if ($1 == 17) print "Visual Studio 17 2022";
              else if ($1 == 16) print "Visual Studio 16 2019";
              else if ($1 == 15) print "Visual Studio 15 2017";
              else print "";
            }'
        )

        if [ -z "$VS_GENERATOR" ]; then
            echo "❌ Could not detect a supported Visual Studio version via vswhere"
            exit 1
        fi

        echo "✅ Detected Visual Studio Generator: $VS_GENERATOR"

        echo "--- Configuring CMake with Visual Studio Generator ---"
        cmake "$SRC_DIR" \
          -G "$VS_GENERATOR" \
          -A x64 \
          -DCMAKE_PREFIX_PATH="$(cygpath -m "$QT_CMAKE_DIR")" \
          -DCMAKE_CONFIGURATION_TYPES="Release" \
          -DCMAKE_BUILD_TYPE=Release

        echo "--- Building application ---"
        cmake --build . --config Release --parallel

        echo "--- Copying build artifacts ---"
        cp "Release/${APP_NAME}.exe" "$DIST_SUBDIR/" || echo "Warning: EXE not found"

        echo "--- Searching for windeployqt ---"
        QT_BIN_DIR="$(find /c/Qt/6.* -type f -name windeployqt.exe -path "*/msvc*/bin/windeployqt.exe" -print -quit | sed 's:/windeployqt.exe$::')"

        if [ -z "$QT_BIN_DIR" ]; then
            echo "❌ Could not locate windeployqt.exe"
            echo "Please ensure Qt for MSVC is installed under C:/Qt/*"
            exit 1
        fi

        echo "✅ Found windeployqt in: $QT_BIN_DIR"

        echo "--- Running windeployqt to bundle Qt dependencies ---"
        "$QT_BIN_DIR/windeployqt.exe" "$(cygpath -w "$DIST_SUBDIR/${APP_NAME}.exe")"

        cd - > /dev/null

        echo "--- Windows build completed ---"
        echo "Binary available at: ${DIST_SUBDIR}/${APP_NAME}.exe"
        ;;
        
    macos)
        # Apply clean if requested
        if [ "$CLEAN_BUILD" = true ]; then
            clean_platform macos
        fi
        
        # Define directories using absolute paths for clarity
        HOST_DIST_DIR="${PWD}/${DIST_DIR}/macos"
        HOST_BUILD_DIR="${PWD}/${BUILD_DIR}/macos"
        
        echo "--- Starting build for ${PLATFORM} using native Qt ---"
        echo "Host dist directory: ${HOST_DIST_DIR}"
        
        # This approach uses Qt's native tools rather than cross-compiling
        mkdir -p "${HOST_BUILD_DIR}"
        cd "${HOST_BUILD_DIR}"
        
        # Use native macOS tools and Qt
        echo "Using native Qt for macOS build..."
        
        # Get correct source directory path
        SRC_DIR="${PWD}/../../src"
        echo "Source directory: ${SRC_DIR}"
        
        # Run CMake with absolute path to source
        cmake -DCMAKE_BUILD_TYPE=Release "${SRC_DIR}"
        
        # Build the application
        echo "Building application..."
        make -j4
        
        # Create app bundle - CMake already creates the basic structure
        echo "Creating macOS app bundle..."
        
        # For macOS, CMake builds directly into the .app directory
        if [ -f "LongView.app/Contents/MacOS/LongView" ]; then
            echo "Executable found in app bundle, using that"
            APP_BUNDLE_DIR="LongView.app"
        else
            echo "Error: Executable not found in expected location"
            echo "Looking for executable..."
            find . -name "LongView" -type f
            exit 1
        fi
        
        # Create Info.plist if not already created by CMake
        if [ ! -f "${APP_BUNDLE_DIR}/Contents/Info.plist" ]; then
            echo "Creating Info.plist..."
            cat > "${APP_BUNDLE_DIR}/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>CFBundleIdentifier</key>
    <string>${APP_ID}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>${APP_VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${APP_VERSION}</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF
        else
            echo "Info.plist already exists"
        fi
        
        # Ensure Resources directory exists
        mkdir -p "${APP_BUNDLE_DIR}/Contents/Resources"

        # Look for icon file
        ICON_PATH="${SRC_DIR}/assets/icons/macOS/icon.icns"
        if [ -f "${ICON_PATH}" ]; then
            echo "Copying app icon from ${ICON_PATH}..."
            cp "${ICON_PATH}" "${APP_BUNDLE_DIR}/Contents/Resources/AppIcon.icns"
        else
            echo "Warning: Icon file not found at ${ICON_PATH}"
            # Create a simple icon placeholder
            touch "${APP_BUNDLE_DIR}/Contents/Resources/AppIcon.icns"
        fi
        
        # Copy app bundle to dist directory - use absolute paths
        echo "Copying app bundle to distribution directory..."
        mkdir -p "${HOST_DIST_DIR}/app"
        
        # Remove existing app bundle
        rm -rf "${HOST_DIST_DIR}/app/${APP_NAME}.app"
        
        cp -R "${APP_BUNDLE_DIR}" "${HOST_DIST_DIR}/app/"
        
        echo "Checking distribution directory contents..."
        ls -la "${HOST_DIST_DIR}/app/" || echo "Directory not found or empty"
        
        # Return to original directory
        cd "${PWD}/../../"
        
        echo "--- macOS build completed ---"
        echo "App bundle available at: ${DIST_DIR}/macos/app/${APP_NAME}.app"
        ;;
        
    *)
        echo "Error: Invalid platform '$PLATFORM'"
        print_usage
        ;;
esac

echo "--- Build process finished for ${PLATFORM} ---"
