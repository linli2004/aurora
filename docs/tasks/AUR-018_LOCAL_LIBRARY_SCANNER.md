# AUR-018 — Local Library Scanner

## Task

Implement the first Local Music Alpha vertical slice: scan a local folder,
persist local track/source records and allow scanned sources to become the
playback queue.

## Context

AUR-016 introduced local Track Identity v0.1. ADR-002 accepted the repository
boundary: Track and TrackSource are separate, SQLite owns persistence, QML reads
semantic service state and sends commands.

## Source of Truth

- `docs/decisions/ADR-002-local-repository-identity-persistence.md`
- `docs/reviews/AUR-016_STABLE_TRACK_IDENTITY.md`
- `docs/engineering/ARCHITECTURE.md`
- `docs/handoff/08_NEXT_TASKS.md`

## In Scope

- SQLite schema creation for `tracks`, `track_sources` and `schema_migrations`.
- Repository upsert for local sources.
- Recursive scanner for local audio file extensions.
- Background scan worker.
- Scan progress count and cancellation.
- QML scan-folder entry in Music Space.
- Load scanned sources into the in-memory playback queue.
- Repository unit tests.

## Out of Scope

- Metadata extraction during scan.
- Artwork cache table.
- File watcher.
- Delete/relink UI.
- Persistent queue.
- Moment links.

## Allowed Files

- `runtime/library/*`
- `runtime/audio/LocalTrackIdentity.*`
- `app/main.cpp`
- `qml/MusicSpace.qml`
- `tests/test_local_library_repository.cpp`
- Build and documentation files required for this task.

## Acceptance Criteria

- Project builds in dev and release.
- Repository creates schema and migration marker.
- Duplicate byte-identical files map to one Track and two TrackSources.
- Music Space can start a folder scan without QML owning repository state.
- Scanned sources can be loaded into the existing playback queue.

## Required Tests

- `cmake --preset dev`
- `cmake --build --preset dev`
- `ctest --preset dev`
- `python3 scripts/check_structure.py`
- `python3 scripts/validate_fixtures.py`
- `cmake --preset release`
- `cmake --build --preset release`

## Visual Evidence

Generate or review `tests/snapshots/AUR-013_handoff_home.png`; this task changes
Music Space controls, not the Home first viewport.

## Rollback

Revert this task commit. The repository database is created under the Qt app data
location and can be ignored or removed during Alpha development.
