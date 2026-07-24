# AUR-022 — Ubuntu Media Integration / MPRIS

## Task

Expose Aurora as a minimal MPRIS-compatible media player on the user D-Bus
session.

## Context

Aurora already has local playback, queue control, metadata, volume and position
state. Ubuntu / GNOME media keys and shell media surfaces need a standard
desktop integration layer instead of QML-only controls.

## In Scope

- Register `org.mpris.MediaPlayer2.aurora` on the session bus.
- Export `/org/mpris/MediaPlayer2`.
- Implement the root `org.mpris.MediaPlayer2` interface.
- Implement the minimal `org.mpris.MediaPlayer2.Player` interface.
- Support Play, Pause, PlayPause, Stop, Previous, Next, Seek and SetPosition.
- Expose PlaybackStatus, Metadata, Volume, Position and capability properties.
- Emit D-Bus `PropertiesChanged` for playback, track, duration, queue and volume
  state changes.
- Keep MPRIS as a platform integration layer around `AudioRuntime`.

## Out of Scope

- TrackList interface.
- Playlists interface.
- Desktop file installation.
- D-Bus activation service file.
- Artwork cache URI normalization.
- Flatpak permissions.
- Lock-screen artwork polish.

## Architecture Constraints

- MPRIS must not own playback state.
- MPRIS must call existing `AudioRuntime` commands.
- QML must not talk directly to D-Bus.
- Failure to register MPRIS must not prevent Aurora from launching.
- The app continues to work without a session D-Bus bus.

## Acceptance Criteria

1. `playerctl -l` lists Aurora as `aurora` or equivalent.
2. GNOME media keys can Play/Pause when Aurora is running.
3. `playerctl -p aurora metadata title` returns the current title.
4. `playerctl -p aurora next` advances the Aurora queue.
5. `playerctl -p aurora previous` moves backward in the queue.
6. `playerctl -p aurora position 30` seeks to approximately 30 seconds.
7. Closing Aurora removes the MPRIS service from the session bus.
8. All existing tests continue to pass.

## Required Validation

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure

python3 scripts/validate_fixtures.py
python3 scripts/check_structure.py

cmake --preset release
cmake --build --preset release
```

## Manual Validation

```bash
playerctl -l
playerctl -p aurora status
playerctl -p aurora metadata title
playerctl -p aurora play-pause
playerctl -p aurora next
playerctl -p aurora previous
playerctl -p aurora position 30
```

If `playerctl` is not installed, use:

```bash
gdbus call --session   --dest org.mpris.MediaPlayer2.aurora   --object-path /org/mpris/MediaPlayer2   --method org.freedesktop.DBus.Properties.Get   org.mpris.MediaPlayer2.Player PlaybackStatus
```
