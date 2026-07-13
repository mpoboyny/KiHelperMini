#!/bin/bash

# 1. Compile the project using all available CPU cores for speed
cmake --build . --config Release --parallel $(nproc)
