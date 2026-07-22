# AUR-016 — Stable Track Identity

## Task

Freeze the first local Track Identity contract before adding a library scanner
or SQLite repository.

## Context

Aurora currently resolves display metadata during playback. Local Library Alpha
needs stable identity fields that can be reused by scanner, queue persistence and
future repository work without moving music identity into QML.

## Source of Truth

- `docs/specifications/Aurora_Initial_Development_Baseline_0.1.md`
- `docs/engineering/ARCHITECTURE.md`
- `docs/handoff/08_NEXT_TASKS.md`

## In Scope

- Add stable local `trackId` and path-scoped `sourceId`.
- Preserve Unicode filename fallback behavior.
- Add `filePath`, `canonicalTitle`, `durationMs` fixture field and availability.
- Add tests for rename/move stability and duplicate file behavior.
- Expose identity fields as read-only runtime properties.

## Out of Scope

- SQLite persistence.
- Library scanning.
- Full-file hashing for large libraries.
- Relink UI.
- Queue persistence.

## Allowed Files

- `runtime/audio/LocalTrackIdentity.*`
- `runtime/audio/AudioRuntime.*`
- `tests/test_local_track_identity.cpp`
- `fixtures/tracks.json`
- `scripts/validate_fixtures.py`

## Architecture Constraints

- C++ owns typed runtime state and identity resolution.
- QML may read identity fields but must not generate or mutate them.
- Crystal remains music identity; Moment remains memory identity.
- No database schema is introduced in this task.

## Acceptance Criteria

- Same file content keeps the same `trackId` across rename or move.
- Duplicate file content shares `trackId` while different paths get different
  `sourceId` values.
- Missing local files remain representable as unavailable fallback identities.
- Fixture validation requires the new local identity fields.

## Required Tests

- `cmake --preset dev`
- `cmake --build --preset dev`
- `ctest --preset dev`
- `cmake --preset release`
- `cmake --build --preset release`

## Visual Evidence

No new visual behavior. Existing AUR-013 snapshot remains the current baseline.

## Rollback

Revert this task's commit. No schema or persisted user data is created.
