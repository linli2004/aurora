# AUR-024 — Installed Runtime Data / First-Run Experience

## Task

Improve Aurora's first-run and installed-runtime behavior so a newly installed
app gives the user a clear path from an empty library to their local music.

## Context

AUR-021 restored playback position, AUR-022 connected Aurora to Ubuntu media
controls, and AUR-023 gave Aurora a desktop identity. The next gap is the first
minute of use after launch or local installation: the library can be empty, but
the UI should explain what to do and offer a one-step scan of the user's Music
folder.

## In Scope

- Expose the platform default Music directory through `LocalLibraryService`.
- Expose `firstRun`, `defaultMusicDirectory`, `defaultMusicDirectoryLabel` and
  `defaultMusicDirectoryAvailable` to QML.
- Add `scanDefaultMusicDirectory()` as a safe backend entry point.
- Improve the Music Space library menu empty-state copy.
- Add a visible first-run action for scanning the Music folder.
- Keep the existing arbitrary-folder scan path available.
- Improve the empty Tracks panel message.
- Update documentation and structure validation.

## Out of Scope

- Import wizard.
- Multi-directory library roots.
- File watcher.
- Delete/relink UX.
- Permissions portal / Flatpak integration.
- Online providers.

## Architecture Constraints

- First-run state is derived from local library state; it is not a new database
  schema.
- The backend owns platform path discovery.
- QML may present first-run actions but must not hard-code platform music paths.
- Scanning continues to use the existing local repository and scanner pipeline.
- Installed and development builds use the same runtime behavior.

## Acceptance Criteria

1. Empty library state shows clear first-run guidance.
2. A first-run action scans the user's default Music folder when available.
3. The user can still choose another folder.
4. If no readable Music folder exists, the UI falls back to folder selection.
5. Existing scan, cancel, play-library and tracks-panel flows keep working.
6. CTest, fixture validation and structure validation remain green.
7. Release build and installed desktop launch remain functional.

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
gtk-launch aurora
```

## Manual Validation

Use a fresh or temporarily renamed app data directory if needed:

```bash
mv "$HOME/.local/share/Aurora" "$HOME/.local/share/Aurora.backup.$(date +%s)"
```

Then launch Aurora, enter Music Space, open Library, and confirm the first-run
copy and scan action appear. Scan Music should scan the default Music directory
when it exists; Choose folder should still open the folder picker.
