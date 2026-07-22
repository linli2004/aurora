# AUR-008 — Aurora Flow Scene

## Goal

Replace the playback screen's purely circular baseline atmosphere with the first dedicated semantic visual scene, while preserving music identity, readability and low-power fallbacks.

## Data Mapping

```text
Audio level     → overall atmosphere presence
Bass            → gravity bloom and broad ribbon scale
Mid             → primary ribbon movement and spatial body
High            → edge texture, rotation and particle presence
Transient       → restrained wavefront expansion
```

The scene does not expose raw visual parameters to the page API. It consumes semantic audio properties already provided by `AudioRuntime`.

## Interaction

- `Scene: Flow` enables the new scene.
- `Scene: Field` shows the prior baseline implementation for comparison.
- `V` toggles between the two modes.
- `D` continues to toggle diagnostics.

## Accessibility and Quality

- Reduced Motion stops phase-based movement and removes live audio reaction.
- Eco disables particles and phase movement while retaining a static semantic atmosphere.
- The central readability veil protects Crystal, title and controls from visual noise.

## Acceptance Criteria

- Flow and Field modes can switch without interrupting playback.
- Bass, mid, high and transient changes produce visibly different behavior.
- No waveform, equalizer or raw FFT UI is introduced.
- Identity remains recognizable when all dynamic effects are disabled.
- The application continues to pass existing runtime and fixture tests.
