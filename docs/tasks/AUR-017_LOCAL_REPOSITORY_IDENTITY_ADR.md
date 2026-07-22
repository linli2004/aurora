# AUR-017 — Local Repository and Identity Persistence ADR

## Task

Define the SQLite repository, identity persistence and migration contract for
Local Music Alpha without implementing the repository.

## Context

AUR-016 created Track Identity Contract v0.1. Before scanner or database code
depends on it, Aurora needs explicit rules for Track/TrackSource separation,
provisional identities, identity algorithm versions, migration, delete semantics
and export/privacy.

## Source of Truth

- `docs/specifications/Aurora_Initial_Development_Baseline_0.1.md`
- `docs/engineering/ARCHITECTURE.md`
- `docs/handoff/08_NEXT_TASKS.md`
- `docs/reviews/AUR-016_STABLE_TRACK_IDENTITY.md`

## In Scope

- ADR.
- Schema draft.
- Migration strategy.
- Identity algorithm versioning.
- Track/TrackSource boundary.
- Provisional identity semantics.
- Artwork cache decision.
- Delete command separation.
- Thread ownership.
- Export and provenance policy.

## Out of Scope

- SQLite repository implementation.
- Scanner implementation.
- QML UI changes.
- New third-party dependencies.
- Runtime behavior changes.

## Allowed Files

- `docs/decisions/ADR-002-local-repository-identity-persistence.md`
- `docs/tasks/AUR-017_LOCAL_REPOSITORY_IDENTITY_ADR.md`
- `docs/handoff/08_NEXT_TASKS.md`
- `docs/reviews/AUR-016_STABLE_TRACK_IDENTITY.md`

## Architecture Constraints

- C++ owns repository and persistence.
- QML remains read-only for domain data.
- Moment data must not bind only to file paths.
- Local paths are privacy-sensitive export data.

## Acceptance Criteria

- Track / TrackSource boundary is explicit.
- Provisional identity semantics are explicit.
- Identity algorithm name and version are persisted.
- Schema version and migration entry point are defined.
- Artwork cache policy is defined.
- Delete semantics are split.
- Thread ownership is defined.
- Export/provenance policy is defined.
- No runtime behavior is modified.

## Required Tests

- `python3 scripts/check_structure.py`
- `python3 scripts/validate_fixtures.py`

## Visual Evidence

None. Documentation-only task.

## Rollback

Mark ADR-002 Superseded before implementation. After repository code exists,
schema changes must use migrations.
