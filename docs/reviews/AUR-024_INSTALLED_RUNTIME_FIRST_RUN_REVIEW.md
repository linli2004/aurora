# AUR-024 — Installed Runtime Data / First-Run Experience Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ First-Run Manual Validation Pending
○ Installed Launch Validation Pending
```

## Implementation Summary

- Added default Music directory discovery to `LocalLibraryService`.
- Added first-run derived state for empty libraries.
- Added a backend `scanDefaultMusicDirectory()` entry point.
- Updated Music Space library controls to show first-run guidance and a default
  Music-folder scan action.
- Kept the explicit folder picker available.
- Updated empty Tracks panel copy.
- Updated README, CHANGELOG and structure validation.

## Architecture Review

The change stays in Local Library runtime and QML presentation. It does not add
schema, alter Track Identity, change playback session state, or create a new
source of truth. First-run is computed from the existing library count and
scanner status.

## Risk Notes

- On some systems the Music directory may not exist or may not be readable.
- Scanning a large Music directory may take time; existing cancellation remains
  the escape path.
- Users who already have library rows will not see first-run copy.
- Installed desktop launch depends on AUR-023 remaining valid.

## Acceptance Evidence Required

- Development build output.
- Full CTest result.
- Fixture and structure validation.
- Release build and local install output.
- Empty-library screenshot or textual confirmation of first-run copy.
- Scan Music and Choose folder manual validation.

## Final Gate

AUR-024 is accepted only after the first-run UI is visible on an empty library
and existing library behavior remains intact.
