# AUR-026 — Persistent Library Roots & Rescan

## Task

Persist the local music folders that the user has added and provide one action
to rescan all remembered roots.

## Context

AUR-024 added first-run scanning and AUR-025 reconciled missing sources and
stable Track relink. Folder selection is still transient: Aurora forgets which
directories define the library and the user must choose them again.

## In Scope

- Persist normalized library root paths in `library.roots.v1`.
- Restore remembered roots at startup.
- Add a backend `rescanLibraryRoots()` operation.
- Keep Add folder available for extending the library.
- Scan all readable remembered roots in one worker operation.
- Report unavailable remembered roots without deleting them.
- Run AUR-025 missing-source reconciliation after a complete rescan.
- Expose root count, unavailable-root count and a concise summary to QML.
- Add repository tests for root persistence and duplicate normalization.

## Out of Scope

- Removing a remembered root.
- Per-root enable/disable controls.
- Filesystem watcher.
- Scan scheduling.
- Flatpak document-portal persistence.
- Per-root scan timestamps in a new database table.

## Architecture Constraints

- `library.roots.v1` is stored through the existing settings boundary.
- QML never reads or writes root paths directly.
- Failed or cancelled Add folder scans do not persist the new root.
- Missing roots remain remembered so removable storage can return later.
- Source reconciliation occurs only after a complete non-cancelled scan.
- Track and TrackSource identity rules from AUR-025 remain unchanged.

## Persistence Contract

```text
library.roots.v1
└── paths: [normalized absolute paths]
```

Paths are normalized, deduplicated and sorted before storage.

## Acceptance Criteria

1. Successfully scanning a folder adds it to remembered roots.
2. Restarting Aurora preserves the remembered root count.
3. Adding the same folder again does not duplicate it.
4. Adding a second folder produces two remembered roots.
5. Rescan all scans files from every readable remembered root.
6. An unavailable root is reported but remains persisted.
7. Cancelled and failed Add folder scans do not persist a new root.
8. Rescan all still performs missing-source reconciliation.
9. Existing library, playback and installed-runtime behavior remains green.

## Required Validation

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure

python3 scripts/validate_fixtures.py
python3 scripts/check_structure.py

rm -rf build/release
cmake --preset release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
```

## Manual Validation

1. Add one music folder and complete the scan.
2. Restart Aurora and confirm `1 folder · <name>` remains visible.
3. Add a second folder and confirm the summary becomes `2 folders remembered`.
4. Move or remove one root temporarily and run Rescan all.
5. Confirm the unavailable-root status appears and available roots still scan.
6. Restore the folder and run Rescan all again.
