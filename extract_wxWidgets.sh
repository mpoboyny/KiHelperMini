#!/bin/bash

mkdir -p ./wxWidgets/3.3.1/
mkdir -p ./wxWidgets-3.3.1_References


7z x -so wxWidgets.7z wxWidgets-3.3.1.tar.bz2 | tar -xjf - -C ./wxWidgets/3.3.1/ --strip-components=1

7z x -so wxWidgets.7z wxWidgets-3.3.1-docs-html.tar.bz2 | tar -xjf - -C ./wxWidgets-3.3.1_References --strip-components=1

echo Extraction complete: ./wxWidgets/3.3.1 and ./wxWidgets-3.3.1_References
