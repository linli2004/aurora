# AUR-006 — Audio-Reactive Atmosphere

## Status

Initial implementation / Ubuntu validation pending.

## Goal

Make Aurora Atmosphere respond to decoded local audio while preserving the product rule that visual motion serves music rather than becoming a conventional spectrum display.

## Runtime Flow

```text
QMediaPlayer
    ↓
QAudioBufferOutput
    ↓
AudioFeatureAnalyzer
    ↓
Semantic Audio Features
    ↓
AtmosphereField
```

## Semantic Features

- `audioLevel`: overall perceptual presence.
- `bassEnergy`: low-frequency spatial weight.
- `midEnergy`: primary atmosphere body.
- `highEnergy`: edge detail and light movement.
- `transientEnergy`: short-lived wavefront response.

These are prototype semantic estimates based on lightweight filter bands. They are not production mastering measurements and are not exposed as a spectrum UI.

## Experience Mapping

```text
Bass       → field scale and spatial weight
Mid        → secondary field presence
High       → edge movement and fine atmosphere
Transient  → short wavefront expansion
Level      → total atmosphere presence
```

## Accessibility and Quality

- Reduced Motion, High Contrast and Cognitive Minimal do not run continuous audio-driven motion.
- Eco quality keeps identity and readable state while disabling reactive effects.
- If decoded buffers are unavailable, Aurora retains the existing calm ambient fallback.

## Validation

1. Play local music and confirm the fields change without flashing.
2. Pause and confirm the atmosphere returns to quiet instead of freezing at peak energy.
3. Test a bass-heavy and a bright track; the visual weight should differ.
4. Toggle Reduced Motion and Eco quality; continuous reaction must stop.
5. Confirm Core, Crystal and Moment identity remain independent of audio feature values.
