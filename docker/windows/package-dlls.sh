#!/bin/bash
set -e

# Step 1: Locate the .exe
echo "🔍 Locating .exe in build/bin..."
EXE=$(find build/bin -maxdepth 1 -type f -iname '*.exe')
if [ ! -f "$EXE" ]; then
  echo "❌ No .exe found in build/bin"
  exit 1
fi
echo "✅ Found executable: $EXE"

# Extract base name and define paths
EXE_NAME=$(basename "$EXE")
BASE_NAME="${EXE_NAME%.exe}"
TAR_NAME="build/${BASE_NAME}.tar.gz"
OUT_DIR="build/package"

echo "📦 Will create archive: $TAR_NAME"

# Step 2: List DLL dependencies
echo "🧩 Extracting DLL dependencies..."
DLL_NAMES=$(x86_64-w64-mingw32-objdump -p "$EXE" | grep 'DLL Name:' | awk '{print $3}' | sort -u)

# Step 3: Known search paths
KNOWN_PATHS=(
  /opt/qt-win64/bin
  /usr/x86_64-w64-mingw32/bin
  /usr/lib/gcc/x86_64-w64-mingw32/13-win32
)

# Step 4: Prepare output folder
echo "📦 Creating output directory: $OUT_DIR"
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Copy the EXE
echo "📥 Copying EXE..."
cp "$EXE" "$OUT_DIR/"

# Copy DLLs
echo "🔍 Searching and copying DLLs..."
for DLL in $DLL_NAMES; do
  FOUND=false
  for DIR in "${KNOWN_PATHS[@]}"; do
    if [ -f "$DIR/$DLL" ]; then
      cp "$DIR/$DLL" "$OUT_DIR/"
      FOUND=true
      break
    fi
  done
  if [ "$FOUND" = false ]; then
    echo "⚠️  $DLL not found in known paths"
  fi
done

# Step 5: Create tar.gz archive
echo "📦 Creating tar.gz archive..."
(cd "$OUT_DIR" && tar czf "../${BASE_NAME}.tar.gz" .)
echo "✅ Packaged: $TAR_NAME"

# Step 6: Cleanup
echo "🧹 Cleaning up temporary directory..."
rm -rf "$OUT_DIR"
