# AUR-022 — Ubuntu Media Integration / MPRIS Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ Automated Tests Pending
○ GNOME / playerctl Validation Pending
```

## Implementation Summary

- Added `MprisService` as a platform integration wrapper around `AudioRuntime`.
- Added Qt DBus as an application dependency.
- Changed `AudioRuntime` registration to a shared singleton instance so QML and
  MPRIS control the same runtime object.
- Registered the MPRIS bus name `org.mpris.MediaPlayer2.aurora`.
- Exported `/org/mpris/MediaPlayer2`.
- Implemented root and Player adaptors.
- Routed MPRIS commands to existing runtime operations.

## Architecture Review

MPRIS does not become a source of playback truth. It observes and commands
`AudioRuntime`; runtime state remains centralized in the audio runtime, while
QML still uses the same singleton instance.

## Risk Notes

- Qt DBus development files must be available in the local Qt installation.
- Some shell media surfaces may require a desktop file for icon identity; this is
  deferred.
- Position precision depends on Qt Multimedia backend seek behavior.
- Artwork URLs are passed through from the existing runtime and may need cache
  normalization later.

## Acceptance Evidence Required

- Development configure/build output.
- Full CTest result.
- Release build result.
- `playerctl -l` showing Aurora.
- `playerctl -p aurora metadata title`.
- External Play/Pause, Next, Previous and Seek validation.

## Final Gate

The task is accepted only after Ubuntu Release validation confirms external
media control works without regressing in-window playback.
