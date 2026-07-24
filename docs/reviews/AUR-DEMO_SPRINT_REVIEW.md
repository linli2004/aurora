# AUR-DEMO — Presence First Demo Sprint Review

## Status

Accepted for local demo.

## Implemented

- Music Space Presence / Control mode state machine.
- Mouse and keyboard reveal behavior with four-second auto-hide.
- Control chrome fades and scales out instead of remaining fixed.
- Diagnostics are removed from the demo-facing Music Space surface.
- Music Space Hero Crystal receives restrained audio-reactive pulse, tilt and
  edge light.
- Music identity field connects Crystal and Atmosphere visually.
- Home language is Moment-first and Component Gallery is hidden from the default
  demo entry.
- Identity transition and track transition now use Hero Crystal styling in Music
  Space.

## Known Limits

- Visual validation is snapshot/smoke level in automation; final demo still
  requires interactive review on the target Ubuntu display.
- Presence Mode is implemented in QML as the demo contract; production motion
  tokens are still deferred.
- Hero Crystal is Demo-level, not a Production Crystal primitive.

## Validation

- Dev build passed.
- Full dev test suite passed.
- Structure and fixture checks passed.
- Home snapshot generated: `tests/snapshots/AUR-DEMO_home.png`.
- Music Presence snapshot generated:
  `tests/snapshots/AUR-DEMO_music_presence.png`.
- Release build passed.
- Release short launch passed by staying alive until the 5 second timeout.
