# AUR-016 — Stable Track Identity

## Summary

AUR-016 freezes the first local Track Identity contract without introducing a
database or scanner.

## Implemented contract

- `trackId`: stable local content identity using bounded first/last 64 KiB file
  sampling plus file size.
- `sourceId`: path-scoped local source identity using canonical file path.
- `filePath`: canonical local file path when available.
- `canonicalTitle`: case-folded display title for future search/index work.
- `availability`: `Available` or `Unavailable` for the current local source.

## Behavior

- Renaming or moving the same file preserves `trackId`.
- Copying duplicate file content preserves `trackId` while producing a different
  `sourceId`.
- Missing files remain representable as unavailable fallback identities; their
  `trackId` falls back to the path-scoped `sourceId` until content is available.

## Out of scope

- SQLite schema.
- Full-file hash policy for large libraries.
- File watcher or scanner.
- Relink UX.
- Persistent queue.

## Validation

- `cmake --preset dev`
- `cmake --build --preset dev`
- `ctest --preset dev`
- `cmake --preset release`
- `cmake --build --preset release`
- Release short launch with `timeout 5s ./build/release/aurora`
