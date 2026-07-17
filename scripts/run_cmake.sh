#!/bin/bash

LLAMA_SRC_DIR=<!-- llama_src_dir -->
LLAMA_CUDA_FLAG=<!-- llama_cuda_flag -->
LLAMA_OUT_DIR=<!-- llama_out_dir -->

if [ "$LLAMA_CUDA_FLAG" = "ON" ]; then
    LLAMA_BUILD_DIR="build_withCuda"
else
    LLAMA_BUILD_DIR="build_noCuda"
fi

# Navigate to the source directory, exit immediately if the directory does not exist
cd "$LLAMA_SRC_DIR" || exit 1

# Trick CMake's version detection: Create a dummy .git structure 
# to suppress hardcoded Git warnings when building from a static ZIP/folder.
mkdir -p .git
touch .git/index

# Create a dedicated build directory and enter it (Out-of-Source Build)
mkdir -p "$LLAMA_BUILD_DIR"
cd "$LLAMA_BUILD_DIR" || exit 1

# Configure the project using CMake
cmake .. \
    -DCMAKE_INSTALL_PREFIX="$LLAMA_OUT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DGGML_CUDA=$LLAMA_CUDA_FLAG \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
    -DCMAKE_CXX_FLAGS="-finput-charset=UTF-8 -fexec-charset=UTF-8" \
    -DLLAMA_CURL=OFF

