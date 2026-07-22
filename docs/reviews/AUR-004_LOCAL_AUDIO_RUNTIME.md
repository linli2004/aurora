# AUR-004 — Local Audio Runtime v0.2

## Goal

Connect the existing Aurora experience prototype to real local audio without introducing provider, memory or intelligence complexity.

## Implemented

- C++ `AudioRuntime` backed by `QMediaPlayer` and `QAudioOutput`.
- Local multi-file selection through Qt Quick `FileDialog`.
- Queue replacement, append, previous, next and automatic advance.
- Play, pause, seek, volume and playback error handling.
- Filename-based identity fallback until metadata extraction is introduced.
- `CoreExperienceState` mapping from audio runtime state.
- Music Space timeline, queue counter and local-first empty state.
- Keyboard controls: Space, Left, Right and Ctrl+O.
- Pure queue-model unit tests.

## Non-goals

- Audio feature extraction or FFT.
- Album-art and metadata extraction.
- Persistent local library.
- MPRIS or GNOME integration.
- Online providers.
- Memory creation.

## Acceptance checks

1. Select one or more readable local audio files.
2. First track begins playback.
3. Core changes between Gathering, Playing and Paused.
4. Space toggles playback.
5. Left and Right seek by five seconds.
6. Previous restarts the current track after three seconds, otherwise selects the previous queue item.
7. End of media advances to the next queue item.
8. Missing or unreadable files surface a visible error without crashing.
9. All automated tests pass.
