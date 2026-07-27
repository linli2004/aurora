#!/usr/bin/env bash
set -euo pipefail

pkill -x aurora 2>/dev/null || true

if command -v gtk-launch >/dev/null 2>&1; then
    gtk-launch aurora
else
    exec "${HOME}/.local/bin/aurora"
fi

printf '%s
' "Aurora Demo launched. Press F11 for presentation mode."
