# Aurora Initial Architecture v0.2

## Layer Model

```text
QML Experience Layer
        ↓ user intent / semantic properties
Runtime Services and Resolvers (C++)
        ↓ typed state / read-only properties
Local Audio Runtime + Domain Fixtures
```

The prototype keeps rendering intentionally simple. v0.2 introduces local audio as a C++ runtime service while continuing to defer persistence, provider and shader complexity.

## Primitive Responsibilities

| Primitive | Owns | Does not own |
| --- | --- | --- |
| Core | Runtime/perceptual state | Song identity, personal meaning |
| Moment | Memory identity and confirmed meaning | Playback state, artwork identity |
| Crystal | Music identity and recognition | Personal meaning, playback state |
| Atmosphere | Environmental expression | Identity and persistence |

## Runtime Types

`runtime/AuroraTypes.h` is the initial typed contract exposed to QML as `AuroraTypes` under `Aurora.Runtime 1.0`.

`runtime/AuroraStateMapper` is a resolver stub. It maps lifecycle and availability into a human-facing Moment Experience State without placing database lifecycle values directly in QML variants.

`runtime/audio/AudioRuntime` owns `QMediaPlayer`, `QAudioOutput` and the in-memory queue. QML sends playback intent and reads semantic properties; it does not own the media backend.

## QML Contract

QML components expose semantic properties:

- `experienceState`
- `context`
- `accessibilityMode`
- `qualityMode`
- `motionMode`
- `presenceLevel`

They emit intent signals or invoke runtime commands:

- `requestPlayPause()`
- `activated()`
- `recallRequested()`
- `keepMomentRequested()`
- `relinkRequested()`

They do not mutate domain state directly.

## Initial Module Choice

The first repository uses one application QML module (`Aurora.App`) plus a C++ runtime URI (`Aurora.Runtime`). This intentionally reduces build complexity during the first vertical slice. Splitting into `Aurora.Foundation`, `Aurora.Data`, `Aurora.Runtime`, `Aurora.Components` and `Aurora.Demo` remains a Stage A refinement once the build is proven on the target Ubuntu environment.

## Quality Degradation

- Eco: no persistent ambient scaling; minimal translucent layers.
- Balanced: subtle ambient motion and layered gradients.
- Immersive: interface placeholder only in v0.1.
- Adaptive: currently resolves to Balanced.

Identity, state and readability must survive all quality modes.

## Data and Provenance

Fixtures are versioned JSON. Automatically derived values include provenance fields. Confirmed meaning is separate from suggested meaning. The prototype intentionally does not infer a user's personal story.

## Local Audio Boundary

```text
Qt Quick FileDialog
        ↓ selected local URLs
AudioRuntime (C++)
        ↓ playback / queue / position / error
Aurora Core semantic state
        ↓
Music Space presentation
```

The local-audio runtime does not scan libraries, infer emotion, create Moments or access online providers. Those remain separate future modules.
