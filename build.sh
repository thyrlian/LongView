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
            "
        
        echo "--- Linux build completed ---"
        echo "AppImage available at: ${DIST_DIR}/linux/${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
        ;;
        
    windows)
        # Apply clean if requested
        if [ "$CLEAN_BUILD" = true ]; then
            clean_platform windows
        fi
        
        echo "Windows build not implemented yet"
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
