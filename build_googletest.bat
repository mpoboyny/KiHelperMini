@echo off
setlocal

call buildEnv.bat
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to initialize Visual Studio command prompt.
    echo Please check the path to VsDevCmd.bat.
    exit /b 1
)

:: --- 1. Parameter Validation ---
if "%~1"=="" (
    echo Usage: %0 [debug^|release]
    exit /b 1
)

set "MODE=%~1"
if /i not "%MODE%"=="debug" if /i not "%MODE%"=="release" (
    echo Invalid parameter: %1. Use 'debug' or 'release'.
    exit /b 1
)

set "BASE_DIR=%CD%\googletest"

:: Use the root CMake project directory
set "SRC_DIR=%BASE_DIR%"

if not exist "%SRC_DIR%\CMakeLists.txt" (
    echo Error: Could not find CMakeLists.txt in %BASE_DIR%.
    exit /b 1
)

if /i "%MODE%"=="debug" (
    set "BUILD_TYPE=Debug"
    set "TARGET_DIR=%BASE_DIR%\debug-static_x64"
    set "SHARED_LIBS=OFF"
) else (
    set "BUILD_TYPE=Release"
    set "TARGET_DIR=%BASE_DIR%\release-shared_x64"
    set "SHARED_LIBS=ON"
)

echo Source Directory: %SRC_DIR%
echo Build Directory:  %TARGET_DIR%

if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
cd /d "%TARGET_DIR%"

:: --- 2. Configure ---
cmake -G "NMake Makefiles" ^
    "%SRC_DIR%" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_SHARED_LIBS=%SHARED_LIBS% ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
    -DCMAKE_SYSTEM_PROCESSOR=AMD64 ^
    -DCMAKE_CXX_FLAGS="/utf-8 /D_CRT_SECURE_NO_WARNINGS"

if %ERRORLEVEL% neq 0 (
    echo Configuration failed.
    exit /b 1
)

:: --- 3. Build ---
echo Starting %BUILD_TYPE% build...
set CL=/MP
nmake

if %ERRORLEVEL% neq 0 (
    echo Build failed.
    exit /b 1
)

:: --- 4. Collect all libraries in one place ---
echo Collecting libraries...
if not exist "lib" mkdir "lib"
if /i "%SHARED_LIBS%"=="ON" (
    for /r %%F in (*.dll) do copy /y "%%F" "lib\" >nul
    for /r %%F in (*.lib) do copy /y "%%F" "lib\" >nul
    echo Shared libraries are now in: %TARGET_DIR%\lib
) else (
    for /r %%F in (*.lib) do copy /y "%%F" "lib\" >nul
    echo Static libraries are now in: %TARGET_DIR%\lib
)

echo Build completed successfully.
exit /b 0
