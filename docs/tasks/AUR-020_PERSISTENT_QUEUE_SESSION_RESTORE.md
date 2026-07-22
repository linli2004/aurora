# AUR-020 — Persistent Queue and Session Restore

## Task

Implement the minimal playback session persistence needed for the Local Alpha
demo.

## In Scope

- Store queue URLs in the local repository settings table.
- Store current queue index.
- Store volume.
- Restore queue, current source and volume on startup without autoplay.

## Out of Scope

- Playback position persistence.
- Scene persistence.
- Transition personality persistence.
- Crash-safe in-flight playback recovery.
- User-facing settings UI.

## Acceptance Criteria

- Restarting Aurora restores the previous queue context.
- Restore does not autoplay.
- Persistence goes through C++ repository boundaries.

## Required Tests

- Repository settings round trip.
- Dev build and full dev test suite.
- Release build and short launch.

## Rollback

Remove the settings write/read path and ignore the `playback.session.v1` setting.
