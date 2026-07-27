#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${repo_root}"

cmake --preset dev
cmake --build --preset dev --parallel 1
ctest --preset dev --output-on-failure
python3 scripts/validate_fixtures.py
python3 scripts/check_structure.py

cmake --preset release -DCMAKE_INSTALL_PREFIX="${HOME}/.local"
cmake --build --preset release --parallel 1
cmake --install build/release --prefix "${HOME}/.local"

printf '%s
' "Aurora Demo preflight passed."
printf '%s
' "Run scripts/run_demo.sh and press F11 for presentation mode."
