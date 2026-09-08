#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -f "${SCRIPT_DIR}/../KiHelperMini" ]; then
	echo "Error: file not found: ${SCRIPT_DIR}/../KiHelperMini" >&2
	exit 1
fi

if ! command -v ldd >/dev/null 2>&1; then
	echo "Error: ldd program not found" >&2
	exit 1
fi

OUTPUT="$("${SCRIPT_DIR}/../KiHelperMini" 2>&1)"
MISSING_LINE="$(printf '%s\n' "${OUTPUT}" | grep -iE 'error while loading shared libraries|\.so[^[:space:]]*.*(not found|cannot open shared object file)' | head -n 1)"

if [ -n "${MISSING_LINE}" ]; then
	MISSING_SO="$(printf '%s\n' "${MISSING_LINE}" | grep -oE '[^[:space:]:]+\.so(\.[0-9]+)*' | head -n 1)"
	if [ -n "${MISSING_SO}" ]; then
		echo "${MISSING_SO}" >&2
	fi
	echo "${MISSING_LINE}" >&2
	exit 1
fi

echo "All necessary shared libraries available"
