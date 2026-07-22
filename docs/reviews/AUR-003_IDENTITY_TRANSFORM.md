# AUR-003 — Identity-Preserving Transform

## Goal

Prove the first integrated Aurora flow:

```text
Aurora Moment
    ↓
Moment Boundary Dissolves
    ↓
Aurora Crystal Emerges
    ↓
Aurora Core Gathers
    ↓
Atmosphere Expands
    ↓
Music Space
```

## Implementation

- `AuroraHome.identityAnchorRect()` exposes the Moment music-identity anchor.
- `MusicSpace.identityAnchorRect()` exposes the immersive Crystal target.
- `IdentityTransitionLayer` carries the same artwork and color signature between both contexts.
- `AppShell` uses one normalized progress value for page atmosphere and identity continuity.
- The source and target Crystal instances are hidden while the transition carrier is visible.

## Accessibility

Reduced Motion and Cognitive Minimal use a shorter transition duration and remove the transition halo expansion. Identity recognition remains stable.

## Acceptance Criteria

- The Crystal does not disappear between Home and Music Space.
- The same artwork and color signature remain visible through the transform.
- Home fades as Music Space appears, driven by one transition progress value.
- Aurora Core displays Gathering during the transition.
- Back navigation reverses the same identity path.
- Gallery navigation remains independent.
