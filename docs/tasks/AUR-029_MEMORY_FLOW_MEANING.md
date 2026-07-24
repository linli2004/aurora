# AUR-029 — Memory Flow and Confirmed Meaning

## Product purpose

Move from a single proof-of-concept Moment to the first usable personal memory
surface while preserving Aurora's original philosophy:

```text
One Moment First
→ Memory Flow
→ Recall
→ Music Space
```

## Scope

- Persist and expose all Moments newest first.
- Add a dedicated Memory Flow reachable from Home.
- Select and preview any Moment.
- Recall any Available Moment through the existing Crystal identity transition.
- Return from Music Space to the Memory Flow that initiated Recall.
- Add, edit and clear user-authored Confirmed Meaning.
- Preserve Detached Moments and distinguish unavailable music from lost memory.
- Replace technical persistence wording on Home with natural time language.

## Trust contract

```text
Confirmed Meaning = user-authored text only
```

Aurora may store empty text and may display neutral time context. It must not
invent, summarize or infer what a Moment means to the user.

## Acceptance

1. Keeping several Moments produces several Memory Flow entries.
2. Entries are ordered newest first and persist across restart.
3. Selecting an entry updates the featured Moment.
4. Saving Meaning updates the selected Moment and Home when it is latest.
5. Clearing Meaning stores an empty string, never SQL NULL.
6. Recall uses the selected Moment's stable Track identity and preferred source.
7. Back from a Memory-initiated Recall returns to Memory Flow.
8. Detached Moments remain browseable and direct the user to source management.
9. Development, Release, unit, fixture and structure checks pass.
