#!/bin/bash

LLAMA_ROOT_DIR=<! llama_root_dir -->
LLAMA_ZIP="llama.cpp-master.zip"

cd $LLAMA_ROOT_DIR

mkdir -p ./llama.cpp
unzip -q $LLAMA_ZIP -d ./llama.cpp

echo Extraction complete: ./llama.cpp
