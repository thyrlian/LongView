# ====================================================================== #
# Qt Docker Image for macOS Cross-Compilation
# ====================================================================== #

# Base image
FROM --platform=linux/arm64 ubuntu:24.04

# Maintainer
LABEL maintainer="thyrlian@gmail.com"

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Update and install basic tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    apt-utils \
    ca-certificates \
    curl \
    wget \
    git \
    vim \
    bash-completion \
    sudo \
    lsb-release \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install build tools and dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    clang \
    llvm \
    make \
    cmake \
    perl \
    patch \
    libssl-dev \
    libxml2-dev \
    libbz2-dev \
    liblzma-dev \
    libz-dev \
    zlib1g-dev \
    xz-utils \
    python3 \
    python3-pip \
    python3-venv \
    libyaml-cpp-dev \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set osxcross root directory and SDK version
ENV OSXCROSS_ROOT=/opt/osxcross
ENV OSX_SDK_VERSION=11.3

# Clone osxcross
RUN git clone --depth 1 https://github.com/tpoechtrager/osxcross.git ${OSXCROSS_ROOT}

# Ensure tarballs directory exists
RUN mkdir -p ${OSXCROSS_ROOT}/tarballs

# Copy SDK file - must be present in the same directory as Dockerfile
COPY MacOSX.sdk.tar.xz ${OSXCROSS_ROOT}/tarballs/

# Process SDK file
RUN TMP_SDK_DIR=/tmp/sdkfix && \
    rm -rf ${TMP_SDK_DIR} && \
    mkdir -p ${TMP_SDK_DIR} && \
    tar -xvf ${OSXCROSS_ROOT}/tarballs/MacOSX.sdk.tar.xz -C ${TMP_SDK_DIR} && \
    mv ${TMP_SDK_DIR}/MacOSX.sdk ${TMP_SDK_DIR}/MacOSX${OSX_SDK_VERSION}.sdk && \
    tar -cJvf ${OSXCROSS_ROOT}/tarballs/MacOSX${OSX_SDK_VERSION}.sdk.tar.xz -C ${TMP_SDK_DIR} MacOSX${OSX_SDK_VERSION}.sdk && \
    rm ${OSXCROSS_ROOT}/tarballs/MacOSX.sdk.tar.xz && \
    rm -rf ${TMP_SDK_DIR}

# Build osxcross
RUN cd ${OSXCROSS_ROOT} && \
    UNATTENDED=yes ./build.sh

# Add osxcross to PATH
ENV PATH="${PATH}:${OSXCROSS_ROOT}/target/bin"

# Install aqtinstall in a virtual environment
RUN python3 -m venv /opt/venv && \
    /opt/venv/bin/pip install --upgrade pip && \
    /opt/venv/bin/pip install aqtinstall
ENV PATH="/opt/venv/bin:$PATH"

# Set Qt installation directory and version
ENV QT_MACOS_DIR=/opt/Qt6/mac
ENV QT_VERSION=6.5.0

# Install Qt6 for macOS using aqtinstall
RUN mkdir -p ${QT_MACOS_DIR} && \
    aqt install-qt mac desktop ${QT_VERSION} clang_64 -O ${QT_MACOS_DIR}

# Set environment variables for Qt
ENV Qt6_DIR=${QT_MACOS_DIR}/${QT_VERSION}/macos/lib/cmake/Qt6
ENV CMAKE_PREFIX_PATH=${Qt6_DIR}
ENV PATH=${QT_MACOS_DIR}/${QT_VERSION}/macos/bin:$PATH

# Set working directory
WORKDIR /app

# Default command
CMD ["/bin/bash"]

# ====================================================================== #
# IMPORTANT NOTES:
# 1. This doesn't work without additional steps
# 2. You need to provide a macOS SDK due to Apple's licensing restrictions
# 3. To build this image properly, you need to:
#    a. Obtain a macOS SDK from a macOS system (usually from Xcode)
#    b. Package it with
#        i. one-liner: tar -C /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ -cf - $(ls -1 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ | grep 'MacOSX' | head -1) | xz -z > MacOSX.sdk.tar.xz
#        ii. script: build-macos-sdk-tarball.sh
#    c. Place it in the same directory as this Dockerfile
# ====================================================================== #