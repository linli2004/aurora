# AUR-DEMO — Presence First Demo Sprint

## Task

Make Aurora's first impression feel like a music environment rather than a
player control surface.

## Source

- `Aurora_项目计划与UI方案_v1.0.md`

## Scope

- Music Space defaults to Presence Mode.
- Controls reveal on mouse or keyboard interaction.
- Controls auto-hide after four seconds of quiet.
- Library, Tracks, diagnostics and transport chrome do not permanently occupy
  the screen.
- Main Music Space Crystal uses Demo Hero styling and restrained audio response.
- Atmosphere is strengthened around the current music identity.
- Home uses natural Moment language and no longer presents Component Gallery as
  a default demo action.
- Existing Moment, Memory Flow, playback, library and MPRIS capabilities remain
  intact.

## Out of Scope

- New repository capabilities.
- New scanner/indexer behavior.
- New Intelligence, lyrics, online providers or cloud sync.
- Production shader work.
- Full settings, Privacy Center, Archive/Delete or import/export.

## Acceptance

- At rest, Music Space shows Atmosphere, Hero Crystal, song identity, weak Core
  and minimal progress presence.
- Moving the mouse or using the keyboard reveals Control Mode.
- After four seconds without interaction, Music Space returns to Presence Mode.
- Home presents the latest Moment as the emotional entry point.
- Component Gallery is not visible in the default demo Home.
- Dev tests, structure check, fixture check, snapshot smoke and Release build
  pass.
