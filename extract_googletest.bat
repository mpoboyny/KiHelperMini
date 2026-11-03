@echo off
set PATH_7z="C:\Program Files\7-Zip\7z.exe"

if not exist %PATH_7z% (
    echo Error: 7z.exe not found at %PATH_7z%
    exit /b 1
)

mkdir "googletest" 2>nul

%PATH_7z% x googletest-1.18.0.zip -ogoogletest -y

rem Move contents from the inner wrapper folder to the target (including subdirectories)
xcopy /E /Y /I /Q "googletest\googletest-1.18.0\*" "googletest\" >nul
rd /S /Q "googletest\googletest-1.18.0"

echo Extraction complete: googletest
