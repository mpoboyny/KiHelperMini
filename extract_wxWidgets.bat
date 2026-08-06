@echo off
set PATH_7z="C:\Program Files\7-Zip\7z.exe"

if not exist %PATH_7z% (
    echo Error: 7z.exe not found at %PATH_7z%
    exit /b 1
)

rem Create target directories if they do not exist
mkdir "wxWidgets\3.3.1" 2>nul
mkdir "wxWidgets-3.3.1_References" 2>nul

echo Step 1: Extracting TAR.BZ2 packages from 7z archive...
%PATH_7z% e -y wxWidgets.7z wxWidgets-3.3.1.tar.bz2 -o.\temp_extracted_src >nul
%PATH_7z% e -y wxWidgets.7z wxWidgets-3.3.1-docs-html.tar.bz2 -o.\temp_extracted_docs >nul

echo Step 2: Decompressing BZ2 to TAR...
%PATH_7z% x -y .\temp_extracted_src\wxWidgets-3.3.1.tar.bz2 -o.\temp_extracted_src >nul
%PATH_7z% x -y .\temp_extracted_docs\wxWidgets-3.3.1-docs-html.tar.bz2 -o.\temp_extracted_docs >nul

echo Step 3: Extracting source files from TAR archives...
%PATH_7z% x -y .\temp_extracted_src\wxWidgets-3.3.1.tar -o.\temp_src >nul
%PATH_7z% x -y .\temp_extracted_docs\wxWidgets-3.3.1-docs-html.tar -o.\temp_docs >nul

echo Step 4: Moving files to final directories...
rem Copy directly from temp folders since 7-Zip flattens the root directory during TAR extraction
xcopy /E /Y /I /Q ".\temp_src\wxWidgets-3.3.1\*" ".\wxWidgets\3.3.1\" >nul
xcopy /E /Y /I /Q ".\temp_docs\*" ".\wxWidgets-3.3.1_References\" >nul

echo Step 5: Cleaning up temporary files...
rd /S /Q .\temp_extracted_src >nul 2>&1
rd /S /Q .\temp_extracted_docs >nul 2>&1
rd /S /Q .\temp_src >nul 2>&1
rd /S /Q .\temp_docs >nul 2>&1

echo Extraction complete: wxWidgets\3.3.1 and wxWidgets-3.3.1_References
