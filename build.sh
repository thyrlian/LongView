#!/bin/bash

# Ensure script runs in project root directory
PROJECT_DIR=$(pwd)

echo "🔧 Building LongView app..."

# Create build command
BUILD_CMD="cd /workspace && mkdir -p build && cd build && cmake .. && make -j4"

# Execute build process in Docker container
docker run --rm -v $PROJECT_DIR:/workspace qt6-dev bash -c "$BUILD_CMD"

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Application located at: $PROJECT_DIR/build/bin/LongView"
    echo "You can run this application directly on host machine (Qt libraries required)"
    echo "Or use the following command to run it in Docker:"
    echo "docker run --rm -it -e DISPLAY=\$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v $PROJECT_DIR:/workspace qt6-dev bash -c \"cd /workspace/build && ./bin/LongView\""
    
    # Set executable permission
    chmod +x $PROJECT_DIR/build/bin/LongView 2>/dev/null
else
    echo "❌ Build failed, please check the error messages above"
    exit 1
fi
