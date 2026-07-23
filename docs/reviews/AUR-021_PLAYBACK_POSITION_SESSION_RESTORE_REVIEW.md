# AUR-021 — Playback Position Session Restore Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ Automated Tests Pending
○ Manual Restart Validation Pending
```

## Implementation Summary

- `AudioRuntime` persists `positionMs` in the existing session setting.
- Position writes are throttled through a two-second single-shot timer.
- Pause, stop and runtime destruction force a final session write.
- Startup loads the queue without autoplay and applies the position only after
  media duration is available and the backend reports the media as seekable.
- Restore remains pending until `positionChanged` confirms the requested
  position, with bounded retries for backends that ignore an early seek.
- Pending restore state suppresses premature zero-position writes.
- Existing session records remain backward compatible.

## Architecture Review

The change stays inside Runtime State and repository settings. It does not
change Track Identity, Crystal identity, Moment lifecycle or database schema.

## Risk Notes

- Backend seek precision may vary by codec and container.
- A process killed with `SIGKILL` can lose up to the current persistence
  interval, while the previous saved position remains available.
- Scene and transition-personality continuity remain deferred.

## Acceptance Evidence Required

- Development configure/build output.
- Full CTest result.
- Release build result.
- Manual report showing:
  - a track playing beyond 20 seconds;
  - Aurora closing normally;
  - Aurora reopening paused;
  - the same track and approximate position restored.

## Final Gate

The task may be marked accepted only after Ubuntu Release validation.
