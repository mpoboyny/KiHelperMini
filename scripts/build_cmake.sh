#!/bin/bash

LLAMA_SRC_DIR=<!-- llama_src_dir -->
LLAMA_CUDA_FLAG=<!-- llama_cuda_flag -->

if [ "$LLAMA_CUDA_FLAG" = "ON" ]; then
	LLAMA_BUILD_DIR="$LLAMA_SRC_DIR/build_withCuda"
else
	LLAMA_BUILD_DIR="$LLAMA_SRC_DIR/build_noCuda"
fi

cd "$LLAMA_BUILD_DIR" || exit 1

cmake --build . --config Release --parallel $(nproc)
