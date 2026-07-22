# AUR-007 — Audio Diagnostics & Calibration

## Goal

Prove that decoded audio buffers reach the semantic feature analyzer before building more complex visual worlds.

## Scope

- Optional developer-facing diagnostics panel in Music Space.
- Live semantic meters for level, bass, mid, high and transient presence.
- Explicit decoded-buffer availability and ambient-fallback status.
- Keyboard toggle with `D`.

## Acceptance Criteria

- The panel is hidden by default.
- Selecting Audio Data or pressing `D` toggles it without affecting playback.
- During playback, at least Level and one frequency band visibly change.
- Pausing causes values to decay toward zero.
- If decoded buffers are unavailable, the panel states `Ambient fallback` rather than inventing data.
- Reduced Motion and Eco continue to control rendering independently of diagnostics.

## Deferred

- User-facing equalizer.
- Raw FFT visualization.
- Calibration persistence.
- Per-device acoustic profiling.
