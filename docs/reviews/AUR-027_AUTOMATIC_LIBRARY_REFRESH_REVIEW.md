# AUR-027 — Automatic Library Refresh Review

## Review State

Pending local runtime acceptance.

## Architecture Review

- `LocalLibraryWatcher` owns platform watcher registration and debounce only.
- `LocalLibraryService` remains the scan coordinator.
- Repository identity, source availability and root persistence contracts are
  unchanged.
- Filesystem events request the existing complete-root scan path instead of
  duplicating indexing rules.
- Recursive watch paths are collected in the scan worker and registered on the
  main thread after successful completion.

## Safety Review

- Scans remain serialized through the existing `m_scanning` guard.
- Change bursts are coalesced by a single-shot timer.
- Changes during scanning set one boolean follow-up request rather than
  accumulating unbounded work.
- Cancelled or failed scans do not replace the last valid watch set.
- Unreadable roots remain persisted and are reported by AUR-026 behavior.

## Test Review

Added `aurora.local_library_watcher` coverage for:

- canonical readable-directory normalization;
- duplicate-path elimination;
- ignored missing paths;
- watcher clearing;
- debounce coalescing.

Runtime acceptance still requires adding, moving and removing files inside a
remembered directory while Aurora is open.

## Known Limits

`QFileSystemWatcher` resource limits are platform dependent. A future
production milestone must add overflow diagnostics, scalable watch partitioning
or a journal-backed indexing strategy for very large libraries.
