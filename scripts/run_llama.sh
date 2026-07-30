#!/bin/bash

LLAMA_BIN=<!-- llama_bin -->
LLAMA_PARAM=<!-- llama_param -->
LLAMA_PIPE=<!-- llama_pipe -->

if [ -z "$LLAMA_PIPE" ]; then
    ${LLAMA_BIN} ${LLAMA_PARAM}
else
    ${LLAMA_BIN} ${LLAMA_PARAM} | less
fi
