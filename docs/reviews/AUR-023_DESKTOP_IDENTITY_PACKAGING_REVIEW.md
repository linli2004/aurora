# AUR-023 — Desktop Identity / App Icon / .desktop Packaging Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ Install Validation Pending
○ GNOME Launch Validation Pending
```

## Implementation Summary

- Added Linux desktop packaging files under `packaging/linux`.
- Added a configured `aurora.desktop` install target.
- Added a scalable hicolor SVG app icon install target.
- Added CMake install rules for the `aurora` executable.
- Set Qt's desktop file name to `aurora`.
- Added structure validation for packaging files.
- Updated README and CHANGELOG.

## Architecture Review

The change stays in platform identity and packaging. It does not alter playback,
library persistence, MPRIS behavior, Aurora Crystal identity, Moment memory or
Atmosphere rendering.

## Risk Notes

- `desktop-file-validate`, `gtk-launch`, `gtk-update-icon-cache` and
  `update-desktop-database` may not be installed by default.
- Existing CMake presets may need a fresh Release configure with
  `-DCMAKE_INSTALL_PREFIX="$HOME/.local"` so the desktop file points to the
  intended executable path.
- Some GNOME surfaces may cache icons until the session or shell refreshes.
- This is local desktop integration, not production Linux packaging.

## Acceptance Evidence Required

- Development build output.
- Full CTest result.
- Fixture and structure validation.
- Release build output.
- CMake install output.
- File existence checks for binary, desktop file and icon.
- `gtk-launch aurora` or equivalent manual launch result.
- MPRIS still visible through `playerctl -l` after installed launch.

## Final Gate

AUR-023 is accepted only after local install and desktop launch validation pass
on the Ubuntu target machine.
