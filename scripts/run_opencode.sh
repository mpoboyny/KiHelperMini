#!/bin/bash

OPENCODE_PRJ_DIR="<!-- opencode_prj_dir -->"

if [ ! -d "$OPENCODE_PRJ_DIR" ]; then
	if mkdir -p "$OPENCODE_PRJ_DIR"; then
		echo "Created opencode project directory: $OPENCODE_PRJ_DIR"
	else
		echo "Failed to create opencode project directory: $OPENCODE_PRJ_DIR"
		exit 1
	fi
else
	echo "opencode project directory already exists: $OPENCODE_PRJ_DIR"
fi

# If you click 'Do it', opencode.json in OPENCODE_PRJ_DIR will be overwriten and opencode will running. 
