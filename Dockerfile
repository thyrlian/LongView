# ====================================================================== #
# Qt Docker Image
# ====================================================================== #

# Base image
# ---------------------------------------------------------------------- #
FROM ubuntu:24.04

# Author
# ---------------------------------------------------------------------- #
LABEL maintainer="thyrlian@gmail.com"

ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies for Qt 6 and WebEngine
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-webengine-dev \
    qt6-declarative-dev \
    qt6-l10n-tools \
    qt6-qmltooling-plugins \
    qtcreator \
    libgl1-mesa-dev \
    libxcb-xinerama0 \
    libx11-xcb-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-render-util0-dev \
    libxcb-xinput-dev \
    x11-apps \
    git \
    wget \
    unzip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspace

# Enable X11 forwarding (for GUI apps like QtCreator)
ENV DISPLAY=:0
ENV QT_X11_NO_MITSHM=1

# Default command
CMD ["/bin/bash"]
