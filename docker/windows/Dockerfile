# ====================================================================== #
# Qt Docker Image for Windows Cross-Compilation
# ====================================================================== #

# Base image
FROM ubuntu:24.04

# Maintainer
LABEL maintainer="thyrlian@gmail.com"

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Install required packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    mingw-w64 \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    git \
    curl \
    wget \
    unzip \
    ca-certificates \
    libyaml-cpp-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Qt6 host libraries for native build tools (e.g. moc, rcc)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install system dependencies required to build Qt host (including QtWebEngine)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    libdbus-1-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libfreetype-dev \
    libharfbuzz-dev \
    libpci-dev \
    libxshmfence-dev \
    libx11-xcb-dev \
    libxcb1-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-util-dev \
    libglib2.0-dev \
    libnss3-dev \
    libfontconfig1-dev \
    libudev-dev \
    libdrm-dev \
    libgl1-mesa-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxdamage-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxrandr-dev \
    libxrender-dev \
    libxtst-dev \
    libxkbcommon-dev \
    bison \
    flex \
    gperf \
    nodejs \
    python3-html5lib \
    clang \
    libclang-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the Qt version
ENV QT_BRANCH=6.9
ENV QT_VERSION=6.9.0
ENV QT_SOURCE_DIR=/opt/qt/qt-everywhere-src-${QT_VERSION}
ENV CROSS_PREFIX=x86_64-w64-mingw32

# Download and unzip the Qt6 Windows source package
RUN mkdir -p /opt/qt && \
    curl -L -o /opt/qt/qt-win64.zip https://download.qt.io/archive/qt/${QT_BRANCH}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.zip && \
    unzip /opt/qt/qt-win64.zip -d /opt/qt && \
    rm /opt/qt/qt-win64.zip

# Copy toolchain config
COPY toolchain-mingw64.cmake /opt/toolchain-mingw64.cmake

# Build host Qt
RUN mkdir -p /opt/qt/build-host && cd /opt/qt/build-host && \
    cmake ${QT_SOURCE_DIR} \
      -G Ninja \
      -DCMAKE_INSTALL_PREFIX=/opt/qt-host \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
      -DQT_BUILD_EXAMPLES=OFF \
      -DQT_BUILD_TESTS=OFF && \
    cmake --build . --parallel && \
    cmake --install . && \
    [ -x /opt/qt-host/bin/qmake6 ] && \
    [ -x /opt/qt-host/bin/qtpaths6 ] && \
    [ -f /opt/qt-host/lib/cmake/Qt6Core/Qt6CoreConfig.cmake ] && \
    touch /opt/qt-host/.qt-host-built && \
    cd / && \
    rm -rf /opt/qt/build-host

ENV PATH="$PATH:/opt/qt-host/bin"

# Build and install target Qt for Windows (excluding some modules)
# --------------------------------------------------------------------
# ⚠️ Current approach skips compiling certain Qt modules:
#    - qtactiveqt
#    - qtquick3dphysics
#    - qtwebengine
#
# ❓ Reason:
#    These modules either require Windows-only SDKs,
#    depend on MSVC-specific features (e.g., COM support, WebEngine with Chromium),
#    or are not supported under MinGW cross-compilation.
#
# 💡 Recommendation for future:
#    If you need these modules, build Qt on a native Windows machine with MSVC.
#    After building, copy the resulting Qt installation into this Docker image.
#
#    QtWebEngine especially depends on:
#      - MSVC compiler (Chromium does not support MinGW)
#      - Windows SDK (10.0.26100+)
#      - Proprietary tools such as gn, ninja, Python, etc., in a tightly controlled setup.
# --------------------------------------------------------------------
RUN rm -rf ${QT_SOURCE_DIR}/qtactiveqt \
           ${QT_SOURCE_DIR}/qtquick3dphysics \
           ${QT_SOURCE_DIR}/qtwebengine && \
    mkdir -p /opt/qt/build-win && cd /opt/qt/build-win && \
    cmake $QT_SOURCE_DIR \
      -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=/opt/toolchain-mingw64.cmake \
      -DCMAKE_INSTALL_PREFIX=/opt/qt-win64 \
      -DQT_HOST_PATH=/opt/qt-host \
      -DCMAKE_BUILD_TYPE=Release \
      -DQT_BUILD_EXAMPLES=OFF \
      -DQT_BUILD_TESTS=OFF \
      -DSKIP_qtactiveqt=ON \
      -DSKIP_qtquick3dphysics=ON \
      -DSKIP_qtwebengine=ON \
      -Wno-dev && \
    cmake --build . --parallel && \
    cmake --install . && \
    ls /opt/qt-win64/lib/cmake/Qt6/Qt6Config.cmake && \
    ls /opt/qt-win64/bin/qmake6

# Clean up build directories and Qt source
RUN rm -rf /opt/qt/build-* ${QT_SOURCE_DIR}

# Copy helper script for packaging
COPY package-dlls.sh /usr/local/bin/package-dlls.sh
RUN chmod +x /usr/local/bin/package-dlls.sh

# Set the working directory
WORKDIR /app

# Default shell
CMD ["/bin/bash"]
