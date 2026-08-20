#!/bin/bash

LLAMA_BIN="<!-- llama_bin -->"
LLAMA_PIPE="<!-- llama_pipe -->"
LLAMA_SRV_LOG="<!-- llama_srv_log -->"
LLAMA_ARGS=( <!-- llama_param --> )

if [ -z "$LLAMA_SRV_LOG" ] && [ -z "$LLAMA_PIPE" ]; then
    "${LLAMA_BIN}" "${LLAMA_ARGS[@]}"
elif [ -n "$LLAMA_PIPE" ]; then
    "${LLAMA_BIN}" "${LLAMA_ARGS[@]}" | less
elif [ -n "$LLAMA_SRV_LOG" ]; then
    nohup "${LLAMA_BIN}" "${LLAMA_ARGS[@]}" > "${LLAMA_SRV_LOG}" 2>&1 &
    PID=$!
    disown "$PID" 2>/dev/null
    sleep 1
    if kill -0 "$PID" 2>/dev/null; then
        echo "Server running in background (PID: $PID)"
        echo "Logging to: $LLAMA_SRV_LOG"
        echo ""
        echo "Check process with: ps aux | grep llama-server"
        echo "Please close this window."
    else
        echo "Error: Server failed to start or exited immediately."
        echo "Please check the log file: $LLAMA_SRV_LOG"
        echo ""
        echo "Please close this window."
    fi
else
    echo "Error: Something went wrong. Please check the configuration."
    echo ""
    echo "Please close this window."
fi
