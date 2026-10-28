@echo off

set LLAMA_SRC_DIR=<!-- llama_src_dir -->
set LLAMA_CUDA_FLAG=<!-- llama_cuda_flag -->
set LLAMA_OUT_DIR=<!-- llama_out_dir -->
set VSDEVPATH=<!-- vs_dev_cmd_path -->

if "%LLAMA_CUDA_FLAG%"=="ON" (
    set LLAMA_BUILD_DIR=%LLAMA_SRC_DIR%build_withCuda
) else (
    set LLAMA_BUILD_DIR=%LLAMA_SRC_DIR%build_noCuda
)

:: Create a dedicated build directory and enter it (Out-of-Source Build)
if not exist "%LLAMA_BUILD_DIR%" mkdir "%LLAMA_BUILD_DIR%"
cd /d "%LLAMA_BUILD_DIR%" || exit /b 1

call "%VSDEVPATH%" -arch=x64

:: Configure the project using CMake
cmake .. ^
    -DCMAKE_INSTALL_PREFIX="%LLAMA_OUT_DIR%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=ON ^
    -DGGML_CUDA=%LLAMA_CUDA_FLAG% ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
    -DLLAMA_CURL=OFF ^
    -DCMAKE_CXX_FLAGS="/O2 /Ob2 /DNDEBUG" ^
    -DCMAKE_C_FLAGS="/O2 /Ob2 /DNDEBUG"
