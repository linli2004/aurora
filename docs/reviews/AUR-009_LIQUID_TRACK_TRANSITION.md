# AUR-009 — Liquid Track Transition

## Goal

Implement the first manual song-switching personality without replacing identity with a page fade.

## Contract

```text
Outgoing Crystal remains recognizable
        ↓
Liquid field crosses the midpoint
        ↓
Audio queue changes exactly once
        ↓
Incoming Crystal stabilizes
```

## Scope

- Manual Previous / Next only.
- Two or more local files are required for validation.
- Previous restarts the current track when playback is beyond three seconds, matching the existing audio runtime contract.
- Automatic end-of-media transition remains deferred.

## Accessibility

Reduced Motion uses a short stable-anchor cross-dissolve and suppresses broad moving ribbons.

## Acceptance

- Track changes once per interaction.
- Audio continues automatically after the handoff.
- Outgoing and incoming titles are readable.
- Generated Identity remains trusted when artwork is absent.
- Controls ignore repeated taps while the transition is running.
