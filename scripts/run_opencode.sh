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

if [ -f "$OPENCODE_PRJ_DIR/opencode.json" ]; then
	while true; do
		read -r -p "opencode.json exists, do you want overwrite it (y – yes, n – no, x – exit) " answer
		echo "Input: $answer"
		case "$answer" in
			y|Y)
				echo "opencode.json will be overwritten."
				break
				;;
			n|N)
				echo "opencode.json will not be overwritten."
				break
				;;
			x|X)
				echo "Exiting script."
				exit 0
				;;
			*)
				echo "Invalid input. Nothing will be done."
				break
				;;
		esac
	done
fi
