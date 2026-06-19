@echo off
setlocal

rem --- Setup Visual Studio Environment for x64 ---
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to initialize Visual Studio command prompt.
    echo Please check the path to VsDevCmd.bat.
    exit /b 1
)

rem --- Argument Validation ---
if "%~1"=="" (
    echo Usage: %0 [debug^|release^|clean]
    exit /b 1
)

set "MODE=%~1"
if /i not "%MODE%"=="debug" if /i not "%MODE%"=="release" if /i not "%MODE%"=="clean" (
    echo Invalid parameter: %1. Use 'debug', 'release' or 'clean'.
    exit /b 1
)

set "WX_DIR=%cd%\wxWidgets\3.3.1"

if not exist "%WX_DIR%" (
    echo Error: %WX_DIR% does not exist. Please run extract_wxWidgets.bat first.
    exit /b 1
)

cd "%WX_DIR%"

set "BUILD_REL_DIR=build-win-release-x64u"
set "BUILD_DEB_DIR=build-win-debug-x64u"

if /i "%MODE%"=="clean" (
    echo Cleaning build directories...
    if exist "%BUILD_REL_DIR%" rmdir /s /q "%BUILD_REL_DIR%"
    if exist "%BUILD_DEB_DIR%" rmdir /s /q "%BUILD_DEB_DIR%"
    echo Clean complete.
    exit /b 0
)

set "BUILD_DIR="
if /i "%MODE%"=="debug" set "BUILD_DIR=%BUILD_DEB_DIR%"
if /i "%MODE%"=="release" set "BUILD_DIR=%BUILD_REL_DIR%"

if defined BUILD_DIR (
    echo Using build directory: %BUILD_DIR%
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd "%BUILD_DIR%"
) else (
    echo Internal script error: BUILD_DIR not set for mode %MODE%
    exit /b 1
)

if /i "%MODE%"=="debug" (
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS="/DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS" -DwxUSE_UNICODE=OFF -DwxBUILD_SHARED=ON -DwxUSE_STC=ON -DwxUSE_SYS_LIBS=OFF -DwxBUILD_MONOLITHIC=OFF -DwxBUILD_VENDOR=custom ..
) else (
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS="/DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS" -DwxUSE_UNICODE=OFF -DwxBUILD_SHARED=ON -DwxUSE_STC=ON -DwxUSE_SYS_LIBS=OFF -DwxBUILD_MONOLITHIC=OFF -DwxBUILD_VENDOR=custom ..
)

if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed.
    exit /b 1
)

set CL=/MP
nmake

if %ERRORLEVEL% neq 0 (
    echo Error: NMake build failed.
    exit /b 1
)

echo Build complete.
exit /b 0
