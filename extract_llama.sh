#!/bin/bash

mkdir -p ./llama.cpp

unzip -q llama.cpp-master.zip -d ./llama.cpp
shopt -s dotglob nullglob
mv ./llama.cpp/llama.cpp-master/* ./llama.cpp/
rmdir ./llama.cpp/llama.cpp-master

echo Extraction complete: ./llama.cpp
