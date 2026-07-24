# AUR-025 — Missing Source Detection & Stable Track Relink

## Task

Reconcile local library sources with the filesystem and preserve Track identity
when an audio file is moved to a new path.

## Context

Aurora already separates stable content-based `trackId` from path-scoped
`sourceId`. Moving a file therefore produces a new source ID while preserving
the Track ID, but the repository currently leaves the old source marked
Available and counts it in the library.

## In Scope

- Detect local sources whose files no longer exist or are unreadable.
- Mark missing sources `Unavailable` with reason `Missing`.
- Count only Available tracks and sources in the active library.
- Preserve missing source rows as history rather than deleting them.
- Reuse the existing content Track ID when the moved file is scanned at its new
  path.
- Reconcile missing paths during startup and after a successful complete scan.
- Expose `missingSourceCount` through `LocalLibraryService`.
- Show missing-source information through the existing scan-status area.
- Add repository regression coverage for missing and moved files.

## Out of Scope

- Interactive file picker relink.
- File watcher.
- Automatic directory monitoring.
- Deleting historical source records.
- Updating an already persisted playback queue to a moved path.
- Network-volume retry policy.

## Architecture Constraints

- `trackId` remains content identity.
- `sourceId` remains path identity.
- A moved file creates a new TrackSource but must not create a second Track.
- Missing rows remain provenance/history and are excluded from playable lists.
- Reconciliation must not run after a cancelled or failed scan.
- No database schema migration is required because availability fields already
  exist.

## Acceptance Criteria

1. Removing or moving a known file marks its old source Unavailable.
2. Missing sources disappear from Tracks and Play library.
3. Active `sourceCount` and `trackCount` exclude missing rows.
4. Scanning the moved file at its new path restores one playable Track.
5. The old path remains stored as one missing source.
6. Byte-identical copies at two readable paths remain two Available sources for
   one Track, while Tracks and Play library project one preferred source.
7. Startup reconciliation corrects stale Available rows.
8. Existing tests and installed runtime behavior remain green.

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

1. Scan a folder containing one audio file.
2. Close Aurora and move the file to another folder.
3. Reopen Aurora and confirm the old source is reported missing and is no
   longer playable.
4. Scan the new folder.
5. Confirm the library contains one playable Track at the new path rather than
   two Tracks.
