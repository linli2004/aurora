#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="${1:-1.0}"
dist_root="${root}/dist"
bundle_name="aurora-demo-${version}-linux-x86_64"
stage="${dist_root}/${bundle_name}"
archive="${dist_root}/${bundle_name}.tar.gz"

cd "${root}"
./scripts/demo_preflight.sh

rm -rf "${stage}"
mkdir -p "${stage}/usr" "${stage}/docs"

cmake --install build/release --prefix "${stage}/usr"
cp docs/DEMO_1_0_RUNBOOK.md "${stage}/docs/"
cp docs/DEMO_1_0_RELEASE.md "${stage}/docs/"

cat > "${stage}/run-demo.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${root}/usr/bin/aurora" --presentation "$@"
EOF
chmod +x "${stage}/run-demo.sh"

cat > "${stage}/README.txt" <<EOF
Aurora Music Memory Space · Demo ${version}

Run:
    ./run-demo.sh

The bundle expects the system Qt 6 runtime used by the Ubuntu build machine.
For the complete operator story, read docs/DEMO_1_0_RUNBOOK.md.
EOF

tar -C "${dist_root}" -czf "${archive}" "${bundle_name}"
sha256sum "${archive}" > "${archive}.sha256"

printf 'Created %s\n' "${archive}"
printf 'Created %s\n' "${archive}.sha256"
