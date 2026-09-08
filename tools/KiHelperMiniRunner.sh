#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Target file relative path
RELATIVE_TARGET="${SCRIPT_DIR}/../KiHelperMini"

# Convert to an absolute full path
FULL_PATH_BINARY="$(readlink -f "${RELATIVE_TARGET}")"

# Check if the binary actually exists
if [ ! -f "${FULL_PATH_BINARY}" ]; then
    echo "Error: file not found: ${FULL_PATH_BINARY}" >&2
    exit 1
fi

# Get the directory of the binary to load libraries from that exact folder
BINARY_DIR="$(dirname "${FULL_PATH_BINARY}")"

# Add the binary directory to the library search path
export LD_LIBRARY_PATH="${BINARY_DIR}:${LD_LIBRARY_PATH}"

# Launch the application with absolute path and pass through all arguments
exec "${FULL_PATH_BINARY}" "$@"
