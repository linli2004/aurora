# Aurora AI Execution Plan

## Completed in Bootstrap 0.1

- Repository constitution and source-of-truth map
- Qt/CMake application skeleton
- Typed runtime enums
- Moment state mapper
- Semantic tokens
- Core, Moment and Crystal visual prototypes
- Component Gallery
- Home and Music Space prototype screens
- Mock fixtures
- Initial tests and validation scripts

## Next AI Tasks

### AUR-001 — Run and Repair on Target Ubuntu

Install Qt 6 development packages, configure the project, resolve any environment-specific QML module issues, run tests and capture the first screenshots.

### AUR-002 — Split QML Modules

After AUR-001 is green, split the single QML module into Foundation, Components and Demo modules without changing public behavior.

### AUR-003 — Snapshot Harness

Add deterministic snapshot scenes for the minimum Core/Moment/Crystal state set from the baseline.

### AUR-004 — Read-only Mock Repositories

Replace direct fixture properties with `MockMomentRepository` and `MockCrystalRepository` QObject services.

### AUR-005 — Identity-Preserving Transform

Implement a shared Crystal anchor moving from Moment to Music Space. Reduced Motion uses stable-anchor cross-dissolve.

### AUR-006 — Mock Audio Runtime

Introduce semantic state progression: Idle → Gathering → Playing → Paused, without real audio.

### AUR-007 — Real Local Audio

Only after the visual/runtime slice is stable, add Qt Multimedia local playback behind an Audio Runtime service.
