# AUR-021 — Playback Position Session Restore

## Task

Complete the current local playback session by persisting and restoring the
position of the selected queue item.

## Context

AUR-020 persists queue URLs, current index and volume. Restarting Aurora
therefore restores the selected track but always returns to its beginning.
This breaks listening continuity and leaves the session contract incomplete.

## In Scope

- Persist the current media position in `playback.session.v1`.
- Restore the position only after the media duration is known.
- Keep restart behavior paused; restoration must never autoplay.
- Throttle position persistence so SQLite is not written for every player
  position signal.
- Persist immediately when playback pauses, stops, or the runtime is destroyed.
- Extend repository settings regression coverage.

## Out of Scope

- Scene persistence.
- Transition-personality persistence.
- Playback-state/autoplay persistence.
- Queue item database normalization.
- Cross-device session synchronization.

## Architecture Constraints

- The repository remains the persistence boundary.
- QML does not write session data directly.
- Runtime state remains separate from Track and Memory identity.
- Position is scoped to the current queue index and is not Track metadata.
- Restoration must tolerate old `playback.session.v1` records with no
  `positionMs` field.

## Persistence Contract

```text
playback.session.v1
├── urls
├── currentIndex
├── volume
└── positionMs
```

Missing or invalid `positionMs` values resolve to zero.

## Acceptance Criteria

1. Playing a track beyond its opening seconds and restarting Aurora restores
   the same queue item and approximate position.
2. The restored track remains paused until the user presses Play.
3. Old session records without `positionMs` still restore from zero.
4. Restored position is clamped to the loaded media duration.
5. Continuous playback performs at most one SQLite session write per
   persistence interval.
6. Queue clearing resets pending restore state.
7. Existing playback, queue and library tests remain green.

## Required Validation

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure

cmake --preset release
cmake --build --preset release
```

## Manual Validation

Play a local track for at least 20 seconds, pause it, close Aurora, reopen it,
and confirm the same track returns near the saved position without autoplay.
