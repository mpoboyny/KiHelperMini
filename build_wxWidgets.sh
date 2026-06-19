#!/bin/bash
set -euo pipefail

if [ -z "${1-}" ]; then
    echo "Usage: $0 [debug|release]"
    exit 1
fi

MODE=$(echo "$1" | tr '[:upper:]' '[:lower:]')

if [ "$MODE" != "debug" ] && [ "$MODE" != "release" ]; then
    echo "Invalid parameter: $1. Use 'debug' or 'release'."
    exit 1
fi

WX_DIR="$(pwd)/wxWidgets/3.3.1"

if [ ! -d "$WX_DIR" ]; then
    echo "Error: $WX_DIR does not exist. Please run extract_wxWidgets.sh first."
    exit 1
fi

cd "$WX_DIR"

if [ "$MODE" = "debug" ]; then
    BUILD_DIR="build-gtk-debug-shared_x64"
    echo "Configuring and building wxWidgets in $MODE mode..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    ../configure CXXFLAGS="-std=c++17 -m64" --with-gtk=3 --enable-debug --enable-shared --enable-utf8 --enable-stc --disable-sys-libs
    make -j$(nproc)
elif [ "$MODE" = "release" ]; then
    BUILD_DIR="build-gtk-release-shared_x64"
    echo "Configuring and building wxWidgets in $MODE mode..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    ../configure CXXFLAGS="-std=c++17 -m64" --with-gtk=3 --disable-debug --enable-optimise --enable-shared --enable-utf8 --enable-stc --disable-sys-libs
    make -j$(nproc)
fi
