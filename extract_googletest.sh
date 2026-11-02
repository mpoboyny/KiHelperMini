#!/bin/bash

mkdir -p ./googletest

unzip -q googletest-1.18.0.zip -d ./googletest
shopt -s dotglob nullglob
mv ./googletest/googletest-1.18.0/* ./googletest/
rmdir ./googletest/googletest-1.18.0

echo Extraction complete: ./googletest
