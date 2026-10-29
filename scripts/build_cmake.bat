@echo off

set LLAMA_SRC_DIR=<!-- llama_src_dir -->
set LLAMA_CUDA_FLAG=<!-- llama_cuda_flag -->
set VSDEVPATH=<!-- vs_dev_cmd_path -->

if "%LLAMA_CUDA_FLAG%"=="ON" (
    set LLAMA_BUILD_DIR=%LLAMA_SRC_DIR%\build_withCuda
) else (
    set LLAMA_BUILD_DIR=%LLAMA_SRC_DIR%\build_noCuda
)

cd "%LLAMA_BUILD_DIR%" || exit /b 1

call "%VSDEVPATH%" -arch=x64

set CL=/MP
nmake
