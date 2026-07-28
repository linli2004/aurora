# Aurora Demo 1.0 Runbook

## Build and install

```bash
cd ~/下載/aurora-music-framework
./scripts/demo_preflight.sh
```

## Verify the installed runtime

```bash
./scripts/demo_doctor.sh
```

## Launch

The normal demo command now enters presentation mode automatically:

```bash
./scripts/run_demo.sh
```

Use a windowed development launch with:

```bash
./scripts/run_demo_windowed.sh
```

Optional quality modes:

```bash
AURORA_DEMO_QUALITY=eco ./scripts/run_demo.sh
AURORA_DEMO_QUALITY=immersive ./scripts/run_demo.sh
```

`Esc` leaves presentation mode. `F11` toggles it again.

## 60-second story

1. Open on Home with the latest real Moment.
2. Enter the Moment and keep the artwork identity continuous.
3. Let Control Mode retire into Presence Mode.
4. Move the pointer and briefly reveal the controls.
5. Open Source Studio and play one prepared online track.
6. Let the real artwork and atmosphere settle.
7. Keep the current Moment.
8. Return Home and open Memory Flow to show that the new memory belongs to the user.

## Recording freeze

Do not expose Component Gallery, diagnostics, database paths, raw provider errors, terminal output, or the windowed F11 helper during recording.
