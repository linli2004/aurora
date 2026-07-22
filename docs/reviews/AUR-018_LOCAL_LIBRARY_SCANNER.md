# AUR-018 — Local Library Scanner

## Summary

Implemented the first Local Music Alpha library slice.

## Implemented

- `LocalLibraryRepository`: SQLite schema and Track / TrackSource upsert.
- `LocalLibraryService`: runtime singleton for background folder scanning,
  counts, progress, cancellation, status, errors and playable URL export.
- Music Space library panel with scan-folder and play-library actions.
- Repository tests for schema creation, single source round trip and duplicate
  content with multiple sources.

## Architecture

- QML sends scan/play intent only.
- C++ owns scanner, repository, SQLite connection and identity persistence.
- Scanner runs off the UI thread.
- Playback still uses the existing in-memory queue.

## Known limits

- Progress is currently file-count based, not byte-count based.
- Metadata extraction during scan is filename fallback only.
- Artwork table/cache migration is deferred.
- Delete, relink and source availability refresh UI are deferred.
- Persistent queue remains AUR-020.

## Validation

- `cmake --preset dev`
- `cmake --build --preset dev`
- `ctest --preset dev`
- `python3 scripts/check_structure.py`
- `python3 scripts/validate_fixtures.py`
- `cmake --preset release`
- `cmake --build --preset release`
