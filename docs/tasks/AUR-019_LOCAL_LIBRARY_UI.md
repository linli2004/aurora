# AUR-019 — Local Library UI

## Task

Turn the persisted local library into a usable Local Alpha demo surface in Music
Space.

## Context

AUR-018 can scan folders and persist Track / TrackSource records, but the UI only
loads every scanned source into the queue. A demo needs a browseable list, search
and direct play from a selected track while keeping QML out of repository
ownership.

## In Scope

- C++ read-only track list model backed by the local repository.
- Search text command exposed through `LocalLibraryService`.
- Tracks panel in Music Space.
- Click-to-play behavior for a selected visible track.
- Play current visible list behavior.
- Scanner progress count and cancellation control.

## Out of Scope

- Albums view.
- Recent playback.
- Favorites.
- Delete, forget, relink or filesystem deletion commands.
- Artwork cache browsing.
- Large-library virtualized repository paging.

## Acceptance Criteria

- QML reads a C++ model instead of querying SQLite.
- Scan progress and cancellation are visible from Music Space.
- A scanned library can be searched by title/path hints.
- A clicked track becomes the first item in the playback queue.
- Existing audio transition and playback controls still work.

## Required Tests

- `cmake --preset dev`
- `cmake --build --preset dev`
- `ctest --preset dev`
- `python3 scripts/check_structure.py`
- `python3 scripts/validate_fixtures.py`
- Snapshot smoke test.

## Rollback

Revert the AUR-019 implementation commit. Repository schema additions are
forward-compatible for the Alpha demo.
