#!/usr/bin/env bash
set -euo pipefail

pkill -x aurora 2>/dev/null || true

quality="${AURORA_DEMO_QUALITY:-balanced}"
args=(--presentation)

case "${quality}" in
    eco)
        args+=(--eco)
        ;;
    immersive)
        args+=(--immersive)
        ;;
    balanced)
        ;;
    *)
        printf 'Unknown AURORA_DEMO_QUALITY=%s; using balanced.\n' "${quality}" >&2
        ;;
esac

if [[ -x "${HOME}/.local/bin/aurora" ]]; then
    exec "${HOME}/.local/bin/aurora" "${args[@]}"
fi

if command -v aurora >/dev/null 2>&1; then
    exec aurora "${args[@]}"
fi

printf 'Aurora is not installed. Run ./scripts/demo_preflight.sh first.\n' >&2
exit 1
