# AUR-026 — Persistent Library Roots & Rescan Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ Repository Tests Pending
○ Multi-Root Rescan Validation Pending
```

## Implementation Summary

- Added repository helpers for `library.roots.v1`.
- Normalized, deduplicated and sorted persisted root paths.
- Refactored Local Library scanning to support one or many roots.
- Added Add folder and Rescan all behavior.
- Added remembered-root and unavailable-root runtime properties.
- Retained unavailable roots for removable-storage recovery.
- Kept AUR-025 missing-source reconciliation after complete scans.
- Added repository root-persistence regression coverage.

## Architecture Review

The existing settings table remains the persistence boundary. The QML layer
only presents semantic root state and invokes service commands. No database
schema or Track identity contract changes are introduced.

## Risk Notes

- A temporarily unmounted root is reported unavailable and its missing sources
  may be reconciled as unavailable.
- Root removal UX is deferred, so incorrect roots currently remain remembered.
- Large multi-root libraries still run sequentially in one worker.
- Scan progress is a total file count rather than per-root progress.

## Acceptance Evidence Required

- Full Development and Release build output.
- Complete CTest result.
- Root-persistence test result.
- Restart confirmation for remembered roots.
- Two-root Rescan all confirmation.
- Unavailable-root status confirmation.
- AUR-025 missing-source regression confirmation.

## Final Gate

AUR-026 is accepted only when remembered roots survive restart and Rescan all
correctly processes all readable roots without duplicating Track identity.
