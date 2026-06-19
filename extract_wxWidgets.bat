@echo off
set PATH_7z="C:\Program Files\7-Zip\7z.exe"

if not exist %PATH_7z% (
    echo Error: 7z.exe not found at %PATH_7z%
    exit /b 1
)

mkdir "wxWidgets\3.3.1" 2>nul
mkdir "wxWidgets-3.3.1_References" 2>nul

%PATH_7z% x -so wxWidgets.7z wxWidgets-3.3.1.tar.bz2 | tar -xjf - -C "./wxWidgets/3.3.1" --strip-components=1

%PATH_7z% x -so wxWidgets.7z wxWidgets-3.3.1-docs-html.tar.bz2 | tar -xjf - -C "./wxWidgets-3.3.1_References" --strip-components=1

echo Extraction complete: wxWidgets\3.3.1 and wxWidgets-3.3.1_References
