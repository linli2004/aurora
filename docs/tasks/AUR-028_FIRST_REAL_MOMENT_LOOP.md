# AUR-028 — First Real Moment Loop

## Product purpose

Return Aurora development to the original vertical experience plan:

```text
Home → Moment → Crystal → Core → Atmosphere → Music Space
```

This task replaces the mock-only Moment on Home with the first persistent,
user-created memory object.

## Scope

- Add `MomentRepository` on the existing local SQLite database.
- Add `MomentService` as the read-only QML operation boundary.
- Add **Keep moment** in Music Space and keyboard shortcut `K`.
- Persist stable Track identity, source snapshot, metadata, identity color,
  artwork reference, human time and provenance.
- Present the latest saved Moment on Home.
- Recall the latest Moment through the existing identity-preserving transition.
- Resolve the preferred Available source by stable `track_id`.
- Preserve the Moment as Detached when no source is playable.
- Keep Confirmed Meaning empty unless explicitly supplied by the user.

## Frozen boundaries

```text
Moment = Memory Identity
Crystal = Music Identity
Core = Runtime State
Atmosphere = Environment
```

The Moment stores a reference to music identity. It does not become a player,
artwork container, playlist item or listening-history row.

## Acceptance

1. Keeping a loaded local track creates one persistent Moment.
2. Restarting Aurora preserves the Moment.
3. Home shows the latest saved Moment before the current-session fallback.
4. Activating the Moment enters Music Space through the same Crystal identity.
5. Moving the source and rescanning preserves `track_id` and Recall follows the
   new preferred Available source.
6. Removing every source keeps the Moment visible in Detached state.
7. No system-generated personal meaning is stored.
8. Unit, structure, fixture, Development and Release checks pass.
