#!/usr/bin/env bash
set -euo pipefail

binary="${HOME}/.local/bin/aurora"
database="${HOME}/.local/share/Aurora/Aurora Music Framework/library.sqlite"

printf 'Aurora Demo 1.0 doctor\n'
printf '%-22s %s\n' 'Installed binary:' "${binary}"
printf '%-22s %s\n' 'Memory database:' "${database}"

if [[ ! -x "${binary}" ]]; then
    printf 'FAIL: installed Aurora binary is missing.\n' >&2
    exit 1
fi

if [[ ! -f "${database}" ]]; then
    printf 'WARN: memory database does not exist yet. The first launch will create it.\n'
else
    printf '%-22s %s\n' 'Database size:' "$(du -h "${database}" | cut -f1)"
    if command -v sqlite3 >/dev/null 2>&1; then
        moment_count="$(sqlite3 "${database}" 'SELECT COUNT(*) FROM moments;' 2>/dev/null || printf '?')"
        printf '%-22s %s\n' 'Stored Moments:' "${moment_count}"
    fi
fi

printf '%-22s %s\n' 'Presentation launch:' './scripts/run_demo.sh'
printf '%-22s %s\n' 'Windowed launch:' './scripts/run_demo_windowed.sh'
printf 'READY: Aurora Demo 1.0 runtime is available.\n'
