#!/bin/bash
# Create macOS SDK tarball for cross-compilation

# One-liner version
# tar -C /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ -cf - $(ls -1 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ | grep 'MacOSX' | head -1) | xz -z > MacOSX.sdk.tar.xz

set -euo pipefail

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define the SDK base path
SDK_BASE_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs"

# Verify the SDK base path exists
if [ ! -d "$SDK_BASE_PATH" ]; then
    echo "❌ Error: SDK path not found at $SDK_BASE_PATH"
    exit 1
fi

# Find the first matching MacOSX SDK directory
SDK_DIR=$(ls -1 "$SDK_BASE_PATH" | grep '^MacOSX.*\.sdk$' | head -1)

if [ -z "$SDK_DIR" ]; then
    echo "❌ Error: No MacOSX SDK directory found in $SDK_BASE_PATH"
    exit 1
fi

echo "✅ Found SDK: $SDK_DIR"

# Set output filename with full path to script directory
OUTPUT_FILE="${SCRIPT_DIR}/MacOSX.sdk.tar.xz"

# Create the tar.xz archive with maximum compression
echo "📦 Creating SDK package in ${SCRIPT_DIR}..."
XZ_OPT="-9" tar -C "$SDK_BASE_PATH" -cf - "$SDK_DIR" | xz -z > "$OUTPUT_FILE"

echo "🎉 SDK package created: $OUTPUT_FILE"
echo "👉 Use this archive with Dockerfile (macos) for cross-compilation"