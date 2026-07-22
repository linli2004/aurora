# AUR-019 — Local Library UI Review

## Status

Accepted for Local Alpha demo.

## Summary

AUR-019 adds the first browseable local library surface:

- repository-backed `LocalLibraryTrackModel`;
- search command and visible result count on `LocalLibraryService`;
- Music Space Tracks panel;
- click-to-play queue creation from the selected row;
- scan progress and cancellation.

## Contract

QML owns presentation, search text intent and click commands. C++ owns repository
queries, track model data and playback queue resolution.

## Known Limits

- Search is a simple repository query over canonical title, artist, album and
  path hints.
- Metadata indexing is still minimal.
- Albums, recent playback, favorites and delete/relink commands are deferred.
- The panel is desktop-oriented and hidden on narrow viewports.

## Validation

- Dev build passed.
- Full dev test suite passed.
- Release build and launch validation must pass before handoff completion.
