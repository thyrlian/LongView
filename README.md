# LongView 龍閱

A unified, scrollable one-page dashboard app for managers to efficiently review configurable, multi-source reports and metrics across platforms.

## Meaning & Backstory

<p align="center">
  <strong>一屏阅尽百表事，龙阅洞察千端理</strong><br>
  <em>One screen to review a hundred reports, LongView to uncover a thousand threads.</em>
</p>
<p align="center">
  <img src="assets/banner.png" alt="LongView Banner" width="60%">
</p>

The name **LongView** carries layered meaning:

- **Long** refers both to the **Dragon (龍)** — a traditional Chinese symbol of vision, wisdom, and authority, often used to represent emperors and leaders.  Just like how emperors in ancient China would read through stacks of memorials each day to govern their empire, modern managers are tasked with reviewing countless reports and dashboards to steer their organizations.  It also reflects the **long, scrollable nature** of the app interface — a single, extended view that brings everything into one screen.

- **View** aligns with the Chinese word **“閱” (yuè)**, meaning "to read" or "to review", and shares a similar pronunciation.

Together, **LongView** reflects both the **form** and the **purpose** of the app:
a unified, scrollable interface that empowers managers to comprehensively read, review, and reason through complex reports across multiple domains — just like a modern-day dragon reviewing the affairs of their realm.

## HOWTO

### Development Environment

To set up the development environment and build the application:

1. Build the Docker development image:
```bash
# For Linux build
docker build -t qt6-dev-ubuntu -f docker/ubuntu/Dockerfile docker/ubuntu

# For macOS cross-compilation (experimental)
docker build -t qt6-dev-macos-cross -f docker/macos/Dockerfile docker/macos/

# For Windows cross-compilation (experimental)
docker build -t qt6-dev-windows-cross -f docker/windows/Dockerfile docker/windows/
```

2. Build the application using the provided script:
```bash
# Build for Linux (in a Docker container)
./build.sh --target=linux

# Build for macOS (in macOS native environment - RECOMMENDED)
./build.sh --target=macos

# Build with cleaning previous build artifacts
./build.sh --target=<platform> --clean
```

The compiled application will be available in the platform-specific directory:
- Linux: `dist/linux/LongView-0.1-x86_64.AppImage`
- macOS: `dist/macos/app/LongView.app`

### Experimental: macOS Cross-Compilation from Linux

> **Note**: This method is experimental and may not produce a perfect build. It's strongly recommended to use the native macOS build script above when possible.

To cross-compile for macOS from Linux:

1. Run the macOS cross-compilation Docker container:
```bash
docker run -it --rm -v $(pwd):/app qt6-dev-macos-cross /bin/bash
```

2. Inside the container, build the application:
```bash
# Clean previous build artifacts
rm -rf build && mkdir build

# Configure with CMake for macOS target
cmake -S . -B build \
  -DCMAKE_SYSTEM_NAME=Darwin \
  -DCMAKE_C_COMPILER=arm64-apple-darwin20.4-clang \
  -DCMAKE_CXX_COMPILER=arm64-apple-darwin20.4-clang++ \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DOPENGL_INCLUDE_DIR=/usr/include \
  -DOPENGL_gl_LIBRARY=/usr/lib/aarch64-linux-gnu/libGL.so \
  -DCMAKE_PREFIX_PATH=/opt/Qt6/mac/6.5.0/macos

# Build the application
cmake --build build --config Release
```

3. After cross-compilation, finalize on a macOS machine:
```bash
# Package Qt dependencies and sign the app
"$(which macdeployqt)" build/bin/LongView.app -verbose=2 && codesign --deep --force --verbose --sign - build/bin/LongView.app
```

The macOS app bundle will be available at `build/bin/LongView.app`

### Experimental: Windows Cross-Compilation from Linux

> **Note**: This method is experimental and may not produce a perfect build. It's strongly recommended to use the native Windows build process when possible.

To cross-compile for Windows from Linux:

1. Run the Windows cross-compilation Docker container:
```bash
docker run -it --rm -v $(pwd):/app qt6-dev-windows-cross
```

2. Inside the container, build the application:
```bash
# Clean previous build artifacts
rm -rf build && mkdir build

# Configure the project
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/opt/toolchain-mingw64.cmake \
  -DCMAKE_BUILD_TYPE=Release

# Build the application
cmake --build build --parallel
```

The output `.exe` file will be available at: `build/bin/`

3. After cross-compilation, bundle `.exe` with required DLLs

Run the helper script inside the container to collect DLLs and package everything:

```bash
package-dlls.sh
```

The complete archive `.tar.gz` will be created at: `build/`

You can extract and run this bundle directly on Windows without installation.

## License

Copyright © 2025 [Jing Li](https://github.com/thyrlian)

Released under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

See the [LICENSE file](https://github.com/thyrlian/LongView/blob/main/LICENSE) for full details.
