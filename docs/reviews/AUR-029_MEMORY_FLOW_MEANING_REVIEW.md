# AUR-029 — Memory Flow and Confirmed Meaning Review

## Status

```text
Accepted for local demo
Interactive UX validation still recommended
```

## Architecture review

- `MomentRepository` remains the SQLite persistence boundary.
- `MomentService` projects immutable Moment records into QML-friendly items.
- Selection is runtime state and is not written into Moment identity.
- Confirmed Meaning updates only one explicit user-owned field.
- Memory Flow composes Aurora Moment; it does not replace or fork the primitive.
- Recall uses the selected stable Track ID and preferred Available source.
- Detached memory remains visible even when music cannot currently be played.
- Transition origin and return context remain continuous.

## Validation focus

- Create at least three Moments.
- Verify newest-first collection order.
- Edit and clear Meaning.
- Restart persistence.
- Recall from Memory and return to Memory.
- Move a source and verify stable relink.
- Remove all sources and verify Detached presentation.

## Automated validation

- Dev build passed.
- Full dev test suite passed.
- Structure and fixture checks passed.
- Release build passed.
- Release short launch passed by staying alive until the 5 second timeout.
