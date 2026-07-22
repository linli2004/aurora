# AUR-010 — Continuous Motion and Circular Queue

## Goal

Improve perceived track-switch continuity and remove queue boundary dead ends.

## Motion changes

- One 560 ms overlapping timeline replaces the prior two-stage 980 ms sequence.
- `XAnimator`, `ScaleAnimator`, `OpacityAnimator`, and `RotationAnimator` move transition visuals on the scene graph render thread when Qt Quick uses the threaded render loop.
- Outgoing and incoming Crystal groups are temporarily flattened into layers while moving.
- The player atmosphere remains visible beneath a light veil; the transition no longer resembles a page replacement.
- Reduced Motion remains a 180 ms restrained crossfade.

## Queue changes

- Next from the final track wraps to index 0.
- Previous always navigates; from the first track it wraps directly to the final index.
- End-of-media wraps through the same queue implementation.

## Performance validation

Judge smoothness using a Release build. Debug QML and C++ builds are for diagnostics and may not sustain high-refresh animation.

```bash
cmake --preset release
cmake --build --preset release
./build/release/aurora
```

For render-loop diagnostics only:

```bash
QSG_INFO=1 QSG_RENDER_TIMING=1 ./build/release/aurora
```

The application can match a 120 Hz display only when the monitor, GNOME compositor, graphics driver, and Qt Quick scene graph all present at that refresh rate. The design target is consistent frame pacing, not a hard-coded frame count.
