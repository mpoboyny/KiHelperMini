#!/bin/bash

# Safeguard the binary path in quotes
LLAMA_BIN="<!-- llama_bin -->"
LLAMA_PIPE="<!-- llama_pipe -->"

# FIX: Parse the parameters directly into a Bash array to handle spaces and flags without eval
LLAMA_ARGS=( <!-- llama_param --> )

# Execute using the array expansion syntax which perfectly preserves argument boundaries
if [ -z "$LLAMA_PIPE" ] || [ "$LLAMA_PIPE" = "''" ]; then
    "${LLAMA_BIN}" "${LLAMA_ARGS[@]}"
else
    "${LLAMA_BIN}" "${LLAMA_ARGS[@]}" | less
fi
