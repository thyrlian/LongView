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

# Download and unzip the Qt6 Windows source package
RUN mkdir -p /opt/qt && \
    curl -L -o /opt/qt/qt-win64.zip https://download.qt.io/archive/qt/${QT_BRANCH}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.zip && \
    unzip /opt/qt/qt-win64.zip -d /opt/qt && \
    rm /opt/qt/qt-win64.zip

# Set the cross-compilation toolchain prefix
ENV CROSS_PREFIX=x86_64-w64-mingw32

# Set the Qt source directory
ENV QT_SOURCE_DIR=/opt/qt/qt-everywhere-src-${QT_VERSION}

# Copy toolchain file into the container
COPY toolchain-mingw64.cmake /opt/toolchain-mingw64.cmake

# Build host Qt (used for host tools)
RUN cd /opt/qt && \
    mkdir -p build-host && cd build-host && \
    cmake ../qt-everywhere-src-${QT_VERSION} \
      -G Ninja \
      -DCMAKE_INSTALL_PREFIX=/opt/qt-host \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
      -DQT_BUILD_EXAMPLES=OFF \
      -DQT_BUILD_TESTS=OFF

# Set the working directory
WORKDIR /app

# Default shell
CMD ["/bin/bash"]
