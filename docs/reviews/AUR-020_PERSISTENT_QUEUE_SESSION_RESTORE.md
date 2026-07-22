# AUR-020 — Persistent Queue and Session Restore Review

## Status

Minimal demo subset accepted.

## Summary

Aurora now stores `playback.session.v1` in the local SQLite repository settings
table. The saved state includes queue URLs, current index and volume.

On startup, `AudioRuntime` restores readable local URLs and loads the current
track without autoplay.

## Known Limits

- Playback position is not restored.
- Scene and transition personality are not restored.
- There is no settings UI.
- The setting stores local file URLs, which are privacy-sensitive and must be
  handled by future export controls.

## Validation

- Repository settings round trip test added.
- Full release validation is required before handoff completion.
