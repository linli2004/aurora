# Changelog

## Unreleased — Stable Local Track Identity

- Added the first SQLite-backed local library repository with Track and TrackSource tables.
- Added recursive local folder scanning for supported audio files.
- Added a Music Space library panel for scanning a folder and loading scanned sources into the queue.
- Added scan progress and cancellation controls for long local library scans.
- Added a repository-backed Tracks browser with search and click-to-play behavior.
- Added minimal playback session restore for queue URLs, current index and volume.
- Added playback-position persistence to the existing `playback.session.v1` record.
- Restore waits for media duration before applying the saved position and never autoplays.
- Throttled position writes to avoid updating SQLite for every media-player position signal.
- Fixed startup position restore to wait for seekable media and confirm the applied position.
- Added bounded retry handling for multimedia backends that ignore the first seek request.
- Added minimal MPRIS service registration for Ubuntu media controls.
- Exposed Play/Pause, Next, Previous, Stop, Seek, metadata, volume and playback status over D-Bus.
- Added repository tests for schema creation, TrackSource persistence and duplicate content.
- Added repository tests for track search and settings persistence.
- Added stable local Track ID generation from bounded file-content sampling.
- Added path-scoped Source ID, canonical title, file path and availability fields.
- Exposed local identity fields through the C++ audio runtime as read-only QML properties.
- Updated track fixtures and validation for the Local Library Alpha identity contract.
- Added regression tests for rename/move stability, duplicate content and unavailable fallback identity.

## 0.5.3 — Progressive Carousel Identity Handoff

- Slow the default track transition from 780 ms to 1450 ms so identity change is perceptible instead of feeling like a cut.
- Enlarge the outgoing Crystal before movement, then move it along a shallow carousel arc while it progressively shrinks and disappears.
- Start the incoming Crystal near-invisible at the opposite edge, then grow and fade it in continuously until it settles at the playback anchor.
- Delay destination-content landing until 70% of the timeline and overlap the final overlay fade with a 440–500 ms real-content reveal.
- Keep Reduced Motion on a short stable-anchor cross-dissolve path.

## 0.5.2 — Progressive landing and geometry handoff

- Match transition identity to the exact Music Space crystal geometry.
- Add a continuous overshoot-and-settle landing phase.
- Cross-fade the real player content before removing the overlay.
- Remove the last-frame size/position snap observed in the v0.5.1 recording.

## 0.5.1 — Continuous Motion & Circular Queue

- Reworked Liquid Track Transition onto one overlapping timeline.
- Replaced GUI-thread property animations with Qt Quick Animator types for x, scale, rotation and opacity.
- Removed the two explicit transition pauses and shortened the default handoff to 560 ms.
- Added temporary layer caching for the two Crystal groups during the transition.
- Added circular queue traversal: last → first and first → last. Previous always navigates instead of restarting the current track.
- End-of-media now continues from the final track back to the first track.
- Added queue boundary regression tests.

# Changelog

## 0.1.0-bootstrap — 2026-07-21

- Added AI engineering constitution and source-of-truth map.
- Added Qt 6/QML/CMake application skeleton.
- Added typed runtime enums and Moment state mapper.
- Added semantic token singleton.
- Added Aurora Core, Moment and Crystal prototypes.
- Added Aurora Home, Music Space and Component Gallery.
- Added versioned fixtures, tests and validation scripts.
- Added MVP, architecture, execution plan and bootstrap review documents.

## 0.1.1 — Component Gallery Visual Baseline

- Fixed Qt 6.5+ QML module loading with `loadFromModule()`.
- Enabled QTP0001 through `qt_standard_project_setup(REQUIRES 6.5)`.
- Reworked the Component Gallery into reviewable state panels.
- Added state names, descriptions, counters and deterministic controls.
- Added accessibility mode cycling and reset behavior.
- Refined semantic state expression for Core, Moment and Crystal.
- Reduced Atmosphere background dominance.

## 0.1.2 — Identity-Preserving Vertical Slice

- Added a shared Aurora Crystal transition layer between Home and Music Space.
- Replaced the simple page-only cross-fade with a stable identity anchor transform.
- Added Gathering / Transitioning state expression during context changes.
- Added a Reduced Motion cross-dissolve timing path.
- Preserved the same artwork, color signature and optical anchor across contexts.

## 0.2.0 — Local Audio Runtime

- Added a C++ audio runtime using `QMediaPlayer` and `QAudioOutput`.
- Added local multi-file selection and an in-memory playback queue.
- Added play, pause, previous, next, seek, volume and automatic advance.
- Bound Aurora Core semantic state to real playback state.
- Added local-audio empty, loading and failure states to Music Space.
- Added keyboard controls and queue-model tests.

## 0.2.1 — Local Music Identity

- Added local title, artist, album and track-number metadata resolution.
- Added embedded cover-art extraction and local artwork caching.
- Added artwork-derived semantic identity color.
- Added explicit identity provenance and trusted Generated Identity fallback.
- Preserved artwork and color identity across Home, Transform and Music Space.
- Added LocalTrackIdentity resolver tests.

## 0.3.0 — Audio-Reactive Atmosphere

- Added decoded-audio buffer capture through `QAudioBufferOutput`.
- Added a local semantic feature analyzer for level, bass, mid, high and transient presence.
- Added attack/release smoothing and quiet decay for pause, stop and stale buffers.
- Bound audio features to Atmosphere without exposing raw visual controls.
- Preserved ambient fallback when audio buffers are unavailable, Reduced Motion is enabled or Eco quality is selected.
- Added deterministic low/high-frequency analyzer tests.

## 0.3.1 — Audio Diagnostics & Calibration

- Added an optional live audio diagnostics panel to Music Space.
- Exposed semantic level, bass, mid, high and transient meters without exposing renderer controls.
- Added a decoded-buffer availability indicator and ambient-fallback status.
- Added the `D` keyboard shortcut and a visible Audio Data toggle.
- Kept diagnostics disabled by default so the product experience remains quiet.


## 0.4.0 — Aurora Flow Scene

- Added the first dedicated audio-reactive playback scene beyond the baseline atmosphere circles.
- Added layered semantic ribbons driven by bass, mid and high-frequency energy.
- Added a low-frequency gravity bloom, transient wavefronts and sparse high-frequency particles.
- Added a quiet center protection region so music identity and controls remain readable.
- Added a `Scene: Flow / Scene: Field` comparison control and the `V` keyboard shortcut.
- Preserved Reduced Motion and Eco quality fallbacks without making identity depend on effects.


## 0.5.0 — Liquid Track Transition

- Added the first selectable track-transition personality: Liquid Flow.
- Preserved outgoing and incoming Aurora Crystal identity through manual track changes.
- Added a midpoint audio switch so the visual handoff and playback queue remain coordinated.
- Added a Reduced Motion path that uses a shorter stable-anchor cross-dissolve.
- Added Shift+Left / Shift+Right keyboard track navigation while preserving plain-arrow seeking.
- Kept end-of-media automatic advance unchanged for this prototype; automatic transition orchestration remains deferred.
