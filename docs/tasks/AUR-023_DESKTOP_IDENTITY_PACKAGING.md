# AUR-023 — Desktop Identity / App Icon / .desktop Packaging

## Task

Give Aurora a first-class Linux desktop identity that matches its MPRIS
`DesktopEntry` value and can be installed locally through CMake.

## Context

AUR-022 exposes Aurora to Ubuntu / GNOME through MPRIS. The MPRIS root
interface already reports `DesktopEntry=aurora`, but the application still needs
a matching installed desktop entry and icon so shell surfaces can resolve the
player identity consistently.

## In Scope

- Add `packaging/linux/aurora.desktop.in`.
- Add a scalable SVG app icon under `packaging/linux/aurora.svg`.
- Configure and install `aurora.desktop` through CMake.
- Install the icon into the hicolor scalable app icon theme path.
- Install the `aurora` executable through CMake.
- Set Qt's desktop file name to `aurora`.
- Add structure validation for the new packaging files.
- Document local desktop installation and validation.

## Out of Scope

- Debian packaging.
- AppImage / Flatpak packaging.
- D-Bus activation service.
- Autostart.
- Icon theme variants for every raster size.
- Store metadata and screenshots.

## Architecture Constraints

- Desktop identity remains a platform-integration layer.
- Runtime, Crystal, Moment and Atmosphere responsibilities are unchanged.
- MPRIS continues to report the same `DesktopEntry` basename: `aurora`.
- Packaging must not be required for developer builds to run.

## Acceptance Criteria

1. Development build and tests still pass.
2. Release build still passes.
3. `cmake --install build/release --prefix "$HOME/.local"` installs:
   - `$HOME/.local/bin/aurora`
   - `$HOME/.local/share/applications/aurora.desktop`
   - `$HOME/.local/share/icons/hicolor/scalable/apps/aurora.svg`
4. `desktop-file-validate` passes when available.
5. Launching the installed desktop file starts Aurora.
6. `playerctl -l` still lists Aurora while it is running.
7. GNOME / shell surfaces can associate the app with the `aurora` desktop entry.

## Required Validation

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure

python3 scripts/validate_fixtures.py
python3 scripts/check_structure.py

rm -rf build/release
cmake --preset release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
```

## Manual Validation

```bash
test -x "$HOME/.local/bin/aurora"
test -f "$HOME/.local/share/applications/aurora.desktop"
test -f "$HOME/.local/share/icons/hicolor/scalable/apps/aurora.svg"

desktop-file-validate "$HOME/.local/share/applications/aurora.desktop"
gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" || true
update-desktop-database "$HOME/.local/share/applications" || true
gtk-launch aurora
```
