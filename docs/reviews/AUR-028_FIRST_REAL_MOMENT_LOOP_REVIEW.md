# AUR-028 — First Real Moment Loop Review

## Status

```text
Implementation prepared
Local Ubuntu validation pending
```

## Architecture review

- Memory persistence is isolated under `runtime/memory`.
- QML invokes an operation interface and does not mutate persistence state.
- Stable `track_id` is the durable bridge between Moment and Library.
- Source paths remain replaceable and are resolved at Recall time.
- Missing media does not delete personal memory.
- Crystal identity is prepared before the page transition, avoiding a
  `Moment fade out → player fade in` substitution.
- Confirmed Meaning is never synthesized by Aurora.

## Validation focus

- Keep from Music Space.
- Restart persistence.
- Home latest-Moment presentation.
- Recall transition and playback source.
- Stable relink after moving a file.
- Detached appearance after removing all sources.
- Reduced Motion fallback through the existing transition layer.
