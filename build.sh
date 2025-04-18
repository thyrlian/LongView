#!/bin/bash
# Build script for LongView using Docker for cross-platform compilation and packaging.

set -e # Exit immediately if a command exits with a non-zero status.

# --- Configuration ---
DOCKER_IMAGE="qt6-dev" # Name of your Qt development Docker image
BUILD_DIR="build"
DIST_DIR="dist"
APP_NAME="LongView"
APP_VERSION="0.1" # Example version

# --- Helper Functions ---
print_usage() {
    echo "Usage: $0 <platform>"
    echo "  platform: linux, windows, macos"
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
if [ "$#" -ne 1 ]; then
    print_usage
fi

PLATFORM=$1
CMAKE_PLATFORM_ARGS="" # Platform specific CMake arguments (toolchains, etc.)

# --- Platform Specific Setup ---
case "$PLATFORM" in
    linux)
        clean_platform linux
        
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
            ${DOCKER_IMAGE} /bin/bash -c "
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
        clean_platform windows
        echo "Windows build not implemented yet"
        ;;
        
    macos)
        clean_platform macos
        echo "macOS build not implemented yet"
        ;;
        
    *)
        echo "Error: Invalid platform '$PLATFORM'"
        print_usage
        ;;
esac

echo "--- Build process finished for ${PLATFORM} ---"
