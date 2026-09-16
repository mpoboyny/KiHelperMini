@echo off
setlocal enabledelayedexpansion

REM set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"
REM set "PATH=%CUDA_PATH%\bin;%PATH%"

:: --- 1. Parameter Validation ---
if "%~1"==" " (
    echo Error: Missing parameter.
    echo Usage: %0 [debug^|release^|clean]
    exit /b 1
)

set "MODE=%~1"
if /i not "%MODE%"=="debug" if /i not "%MODE%"=="release" if /i not "%MODE%"=="clean" (
    echo Error: Invalid parameter '%MODE%'.
    echo Usage: %0 [debug^|release^|clean]
    exit /b 1
)

:: --- 2. Initialize VS Environment for x64 Compiler ---
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64

set "CLEAN_MODE=0"
if /i "%MODE%"=="clean" set "CLEAN_MODE=1"

:: --- 3. Check if cmake is available ---
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: 'cmake' was not found. Please install CMake and add it to your PATH.
    pause
    exit /b 1
)

set "BASE_DIR=%CD%\llama.cpp"
set "TARGET_DIR=%BASE_DIR%\msbuild"
set "OUT_DEBUG=%BASE_DIR%\debug-shared_x64_ms"
set "OUT_RELEASE=%BASE_DIR%\release-shared_x64_ms"
set "OUT_TRACE=%BASE_DIR%\trace-shared_x64_ms"

:: --- 4. Perform Cleaning if requested ---
if "%CLEAN_MODE%"=="1" (
    echo Cleaning previous build directories...
    if exist "%TARGET_DIR%" rd /s /q "%TARGET_DIR%"
    if exist "%OUT_DEBUG%" rd /s /q "%OUT_DEBUG%"
    if exist "%OUT_RELEASE%" rd /s /q "%OUT_RELEASE%"
    if exist "%OUT_TRACE%" rd /s /q "%OUT_TRACE%"
    echo Cleanup complete.
    exit /b 0
)

:: --- 5. Find Source Directory ---
set "SRC_DIR="
for /r "%BASE_DIR%" %%F in (CMakeLists.txt) do (
    if exist "%%F" (
        set "SRC_DIR=%%~dpF"
        goto :found
    )
)

:found
if "%SRC_DIR%"=="" (
    echo Error: Could not find CMakeLists.txt in %BASE_DIR%
    pause
    exit /b 1
)

if "%SRC_DIR:~-1%"=="\" set "SRC_DIR=%SRC_DIR:~0,-1%"

:: --- 6. Create build directory ---
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
cd /d "%TARGET_DIR%"

echo Source Directory: %SRC_DIR%
echo Solution Folder:  %TARGET_DIR%
echo Build Mode:       %MODE%

:: --- 7. Configure Solution using NMake Makefiles ---
cmake -G "NMake Makefiles" ^
    "%SRC_DIR%" ^
    -DBUILD_SHARED_LIBS=ON ^
    -DGGML_CUDA=OFF ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
    -DLLAMA_CURL=OFF ^
    -DCMAKE_BUILD_TYPE=%MODE% ^
    -DCMAKE_CXX_FLAGS_TRACE="/O2 /Ob2 /DNDEBUG /D_MPTRACE_" ^
    -DCMAKE_C_FLAGS_TRACE="/O2 /Ob2 /DNDEBUG /D_MPTRACE_" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG="%OUT_DEBUG%" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE="%OUT_RELEASE%" ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_TRACE="%OUT_TRACE%" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG="%OUT_DEBUG%" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE="%OUT_RELEASE%" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_TRACE="%OUT_TRACE%"

if %ERRORLEVEL% equ 0 (
    echo.
    echo Solution configured successfully.
    echo Starting build...
    
    :: Nutzt alle CPU-Kerne für den MSVC-Compiler über NMake
    set CL=/MP
    nmake
) else (
    echo.
    echo Configuration failed.
    exit /b 1
)

echo.
echo Create of llama.cpp complete.
