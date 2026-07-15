@echo off

set LLAMA_ROOT_DIR=<! llama_root_dir -->
LLAMA_ZIP="llama.cpp-master.zip"

set PATH_7z="C:\Program Files\7-Zip\7z.exe"

if not exist %PATH_7z% (
    echo Error: 7z.exe not found at %PATH_7z%
    exit /b 1
)

cd %LLAMA_ROOT_DIR%

mkdir "llama.cpp" 2>nul

%PATH_7z% x llama.cpp-master.zip -ollama.cpp -y

echo Extraction complete: ./llama.cpp
