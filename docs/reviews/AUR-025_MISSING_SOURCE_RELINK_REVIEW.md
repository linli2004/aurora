# AUR-025 — Missing Source Detection & Stable Track Relink Review

## Status

```text
Implementation Prepared
○ Ubuntu Build Pending
○ Repository Regression Tests Pending
○ Move / Relink Manual Validation Pending
```

## Implementation Summary

- Changed active library counts to include Available sources only.
- Added repository missing-source counting.
- Added filesystem reconciliation for Available local source rows.
- Added startup and post-scan reconciliation.
- Added missing-source state to `LocalLibraryService`.
- Preserved stable Track identity across moved paths.
- Added one preferred-source projection per Track for Tracks and Play library.
- Added moved-file and missing-file repository tests.

## Architecture Review

The implementation uses the existing Track / TrackSource model as intended:

```text
Track
= stable content identity

TrackSource
= path-scoped access identity
```

A move creates a new source for the same Track. The inaccessible former source
is retained as Unavailable history.

## Risk Notes

- Reconciliation treats an unreadable or temporarily unmounted local file as
  missing.
- Persisted playback sessions still store URLs and do not automatically rewrite
  a moved queue entry.
- Reconciliation is intentionally filesystem-based and does not monitor changes
  continuously.
- Duplicate readable copies remain separate sources by design.

## Acceptance Evidence Required

- Full Development and Release build output.
- Complete CTest result.
- Missing-file repository test.
- Moved-file stable Track ID test.
- Manual move and rescan result.
- Confirmation that Play library excludes the old path.

## Final Gate

AUR-025 is accepted only when moving one scanned file and rescanning its new
folder results in one playable Track, one Available source and one preserved
missing source.
