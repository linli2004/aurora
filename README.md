# Aurora Music Framework — Initial Development Repository

Aurora is an Ubuntu-native personal music environment built around music identity, memory, atmosphere and continuous experience.

This repository now contains the **Carousel Track Transition Prototype 0.5.3**. It combines the Qt 6 / QML experience shell with a C++ local-audio runtime while preserving the frozen Core, Moment and Crystal boundaries.

## Current Scope

Implemented as an initial scaffold:

- Qt 6 + QML + CMake application skeleton
- Typed runtime enums registered from C++
- Lifecycle → Experience state mapper
- Semantic token singleton
- Aurora Core prototype
- Aurora Moment prototype
- Aurora Crystal prototype
- Component Gallery
- Aurora Home → Music Space identity-preserving navigation
- C++ local audio runtime using QMediaPlayer and QAudioOutput
- Native multi-file selection and an in-memory playback queue
- Play, pause, previous, next, seeking, volume and error states
- Local metadata title, artist, album and track number resolution
- Embedded artwork extraction with a trusted Generated Identity fallback
- Decoded-audio feature analysis for level, bass, mid, high and transient presence
- Audio-reactive Atmosphere with Reduced Motion and Eco fallbacks
- Progressive carousel transition for manual previous/next track changes: outgoing identity enlarges then shrinks away while the incoming identity grows into place
- Artwork-derived semantic identity color and visible provenance
- Mock fixtures and fixture validation
- C++ state-mapping and queue tests

Deferred:

- Persistent metadata index and local library scanning
- Emotion Graph and Intelligence Layer
- Production Memory persistence
- Online music providers
- Final shaders and refraction
- GNOME Shell integration

## Requirements

- CMake 3.21+
- Ninja or Make
- A C++20 compiler
- Qt 6.8+ with Core, Gui, Qml, Quick, Multimedia and Test development packages

On Ubuntu, package names vary by release. A typical starting point is:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-declarative-dev qt6-multimedia-dev \
  qml6-module-qtquick-dialogs
```

## Configure, Build and Run

```bash
cmake --preset dev
cmake --build --preset dev
./build/dev/aurora
```

## First Flow Scene Run

1. Open a Moment and enter Music Space.
2. Select **Choose music** or press `Ctrl+O`.
3. Choose one or more local audio files.
4. Confirm that local metadata appears when available; songs without artwork should keep the trusted Generated Identity.
5. Play the track and observe the Flow Scene respond to the music.
6. Press `V` to compare `Scene: Flow` with the legacy `Scene: Field`.
7. Press `D` to verify decoded audio data while the scene is active.
8. Pause and confirm the atmosphere settles back to quiet.
9. Select at least two files, then use Previous/Next to verify the Liquid Flow identity handoff.
10. Use Shift+Left/Right for track changes; plain Left/Right still seeks by five seconds.

## Run Tests

```bash
ctest --preset dev
```

Fixture and repository checks can run without Qt:

```bash
python3 scripts/check_structure.py
python3 scripts/validate_fixtures.py
```

## AI Development Workflow

Every coding session must begin by reading:

1. `AGENTS.md`
2. `docs/engineering/MVP_v0.1.md`
3. `docs/engineering/ARCHITECTURE.md`
4. The component specification relevant to the task

Use `docs/engineering/TASK_TEMPLATE.md` to create narrowly scoped AI tasks.

## Repository Status

The source, fixtures and patch structure were validated in the generation environment. A real Qt 6.10 build and decoded-buffer behavior must be validated on the target Ubuntu system. Embedded artwork remains optional because Generated Identity is a supported production fallback.

## Audio diagnostics

In Music Space, click **Audio data** or press `D` to inspect live semantic audio features. The panel is a development aid and is hidden by default.
