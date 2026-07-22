# AUR-016 — Stable Track Identity

## Status

Accepted for Local Alpha with conditions.

The implementation is a correct starting point for local library work, but it is
not a permanent audio fingerprint or Production Track Identity.

## Summary

AUR-016 establishes Track Identity Contract v0.1 without introducing a database
or scanner.

## Implemented contract

- `trackId`: bounded local file fingerprint using first/last 64 KiB file
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

## Stability guarantees

- Stable across path rename.
- Stable across path move.
- Stable across byte-identical copies.
- Distinct source identity per canonical path.

## Known limits

- Metadata edits may change `trackId`.
- Container rewrites or remuxing may change `trackId`.
- Transcoding changes `trackId`.
- Missing-file identities are provisional even though the current code uses a
  path-scoped fallback ID.
- Collision policy is not production-final.
- Identity migration, aliasing and merge/split policy are deferred to
  repository design.

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

## AUR-017 follow-up requirements

- Split Track and TrackSource.
- Store identity algorithm name and version.
- Model provisional identity explicitly.
- Define Track Alias migration.
- Define availability reason beyond a single unavailable state.
- Keep `canonicalTitle` as search/grouping hint only, never as primary identity.
