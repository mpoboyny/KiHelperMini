#!/bin/bash

# Check if the first parameter is provided
if [ -z "$1" ]; then
    echo "Usage: $0 [debug|release] [cuda|nocuda]"
    exit 1
fi

MODE=$(echo "$1" | tr '[:upper:]' '[:lower:]')
BASE_DIR="$(pwd)/llama.cpp"

# Determine CUDA flag (default OFF). Accepts: cuda|on|cuda=on -> ON; nocuda|off|cuda=off -> OFF
DGGML_CUDA="OFF"
if [ -n "$2" ]; then
    case "$2" in
        cuda|on|cuda=on|cuda:on)
            DGGML_CUDA="ON"
            ;;
        nocuda|off|cuda=off)
            DGGML_CUDA="OFF"
            ;;
        *)
            echo "Warning: unrecognized CUDA option '$2' — defaulting to OFF"
            DGGML_CUDA="OFF"
            ;;
    esac
fi

# Suffix for target directory based on CUDA choice: 'cuda' or 'ncuda'
if [ "$DGGML_CUDA" = "ON" ]; then
    CUDA_SUFFIX="cuda"
else
    CUDA_SUFFIX="ncuda"
fi

# 1. Find the real source directory
SRC_DIR=$(find "$BASE_DIR" -maxdepth 2 -name "CMakeLists.txt" -exec dirname {} \;)

if [ -z "$SRC_DIR" ]; then
    echo "Error: Could not find CMakeLists.txt in $BASE_DIR."
    exit 1
fi

if [ "$MODE" == "debug" ]; then
    BUILD_TYPE="Debug"
    TARGET_DIR="$BASE_DIR/debug-static_x64-$CUDA_SUFFIX"
    SHARED_LIBS="OFF"
elif [ "$MODE" == "release" ]; then
    BUILD_TYPE="Release"
    TARGET_DIR="$BASE_DIR/release-shared_x64-$CUDA_SUFFIX"
    SHARED_LIBS="ON"
else
    echo "Invalid parameter: $1. Use 'debug' or 'release'."
    exit 1
fi

echo "Source Directory: $SRC_DIR"
echo "Build Directory:  $TARGET_DIR"

mkdir -p "$TARGET_DIR"
cd "$TARGET_DIR"

# 2. Configure
cmake "$SRC_DIR" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_SHARED_LIBS=$SHARED_LIBS \
    -DGGML_CUDA=$DGGML_CUDA \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
    -DCMAKE_CXX_FLAGS="-finput-charset=UTF-8 -fexec-charset=UTF-8" \
    -DLLAMA_CURL=OFF

# 3. Build
if [ $? -eq 0 ]; then
    echo "Starting $BUILD_TYPE build..."
    cmake --build . --config $BUILD_TYPE -j $(nproc)
    BUILD_EXIT=$?
    if [ $BUILD_EXIT -ne 0 ]; then
        echo "Build failed."
        exit 1
    fi

    # 4. Sammle alle Libs an einem Ort für das Makefile
    echo "Collecting libraries..."
    mkdir -p lib
    if [ "$SHARED_LIBS" == "ON" ]; then
        find . \( -name "*.so" -o -name "*.so.*" -o -name "*.a" \) -exec cp -a {} ./lib/ \;
        echo "Shared libraries are now in: $TARGET_DIR/lib"
    else
        find . -name "*.a" -exec cp {} ./lib/ \;
        echo "Static libraries are now in: $TARGET_DIR/lib"
    fi

    echo "Build completed successfully."
else
    echo "Configuration failed."
    exit 1
fi
