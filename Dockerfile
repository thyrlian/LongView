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

# Install basic build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libgl1-mesa-dev \
    git \
    wget \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install Qt dependencies
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-webengine-dev \
    qt6-declarative-dev \
    qt6-l10n-tools \
    qt6-qmltooling-plugins \
    qtcreator \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install X11 dependencies for Qt GUI
RUN apt-get update && apt-get install -y \
    libxcb-xinerama0 \
    libx11-xcb-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-render-util0-dev \
    libxcb-xinput-dev \
    x11-apps \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install AppImage tools and dependencies
RUN apt-get update && apt-get install -y \
    fuse \
    file \
    appstream \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install AppImageKit for Linux packaging
RUN mkdir -p /opt/appimage && \
    cd /opt/appimage && \
    wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage && \
    chmod +x appimagetool-x86_64.AppImage && \
    ./appimagetool-x86_64.AppImage --appimage-extract && \
    mv squashfs-root appimagetool && \
    ln -s /opt/appimage/appimagetool/AppRun /usr/local/bin/appimagetool

# Set the working directory
WORKDIR /app

# Enable X11 forwarding (for GUI apps like QtCreator)
ENV DISPLAY=:0
ENV QT_X11_NO_MITSHM=1

# Default command - we'll mount the script instead of copying it
CMD ["/bin/bash", "/app/docker-build.sh"]
