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
- Stable local Track ID, path-scoped Source ID, canonical title and availability fields
- SQLite-backed local library repository with Track / TrackSource persistence
- Recursive local folder scan for supported audio files
- Scanner progress, cancellation and repository-backed track list refresh
- Button-expanded local library browser with search and click-to-play queue creation
- Playback session restore for queue, current index, volume and track position
- Minimal MPRIS integration for Ubuntu / GNOME media controls
- Desktop identity packaging with a local `.desktop` entry and hicolor SVG icon
- First-run library guidance with one-step Music folder scanning
- Missing-source reconciliation with stable Track relink after file moves
- Persistent local library roots with multi-folder Rescan all
- Debounced automatic library refresh for remembered folders while Aurora is running
- Presence-first Music Space demo: controls reveal on interaction and return to
  a quiet music environment after idle
- Demo Hero Crystal with restrained audio-reactive pulse, tilt and edge light
- Moment-first Home entry with Component Gallery hidden from the default demo
- Embedded artwork extraction with a trusted Generated Identity fallback
- Decoded-audio feature analysis for level, bass, mid, high and transient presence
- Audio-reactive Atmosphere with Reduced Motion and Eco fallbacks
- Progressive carousel transition for manual previous/next track changes: outgoing identity enlarges then shrinks away while the incoming identity grows into place
- Artwork-derived semantic identity color and visible provenance
- Mock fixtures and fixture validation
- C++ state-mapping and queue tests

Deferred:

- Full metadata index refinement, albums view, root removal, delete commands and interactive relink UX
- Production-scale watcher overflow recovery and per-file incremental indexing
- Full persistent experience state including scene and transition personality
- Emotion Graph and Intelligence Layer
- Production Memory persistence
- Online music providers
- Final shaders and refraction
- Deeper GNOME Shell integration, desktop activation and packaging

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

## Aurora Demo Flow

1. Open Aurora on Home and start from the latest Moment.
2. Click the Moment to recall it into Music Space.
3. Let Music Space settle into Presence Mode: Atmosphere, Hero Crystal, song
   identity and a quiet Core remain visible.
4. Move the mouse to reveal Control Mode, then wait for controls to retreat.
5. Use **Keep moment** or press `K` while a local track is loaded.
6. Return Home and confirm the latest Moment has become the entry point.

## First Flow Scene Run

1. Open a Moment and enter Music Space.
2. Select **Choose music** or press `Ctrl+O`.
3. Choose one or more local audio files.
4. Confirm that local metadata appears when available; songs without artwork should keep the trusted Generated Identity.
5. Play the track and observe the Flow Scene respond to the music.
6. Press `V` to compare `Scene: Flow` with the legacy `Scene: Field`.
7. Pause and confirm the atmosphere settles back to quiet.
8. Select at least two files, then use Previous/Next to verify the Liquid Flow identity handoff.
9. Use Shift+Left/Right for track changes; plain Left/Right still seeks by five seconds.

## Local Library Demo Flow

1. Open a Moment and enter Music Space.
2. Press `Ctrl+L` or open **Library** and select **Add folder**.
3. Choose a local music directory and watch the scan count update.
4. Add another folder, restart Aurora, and confirm the remembered-folder summary remains visible.
5. Select **Rescan all** to scan every remembered readable root and reconcile missing sources.
6. Add, move or remove an audio file inside a remembered readable root and confirm the library refreshes automatically after the debounce window.
7. Use **Cancel scan** during a long scan if needed.
8. Open **Tracks**, search the library, click a row to play from that track, or select **Play list**.
9. Play beyond the opening seconds, restart Aurora, and confirm the queue, current track, volume and position return without autoplay.
10. On a fresh library, open Library and choose Scan Music or Choose folder to start first-run setup.
11. While a track is loaded, run `playerctl -l` and confirm Aurora is exposed as an MPRIS player.

## Local Desktop Install

Configure the Release build with a local prefix, then install Aurora into the
current user's desktop environment:

```bash
rm -rf build/release
cmake --preset release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Optional desktop database refresh:

```bash
desktop-file-validate "$HOME/.local/share/applications/aurora.desktop"
gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" || true
update-desktop-database "$HOME/.local/share/applications" || true
gtk-launch aurora
```

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
