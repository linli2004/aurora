# AUR-012 — Progressive Carousel Identity Handoff

## User-observed problem

v0.5.2 removed the final geometry snap, but the whole track transition still
felt too fast. The visual identity appeared to be replaced rather than lived
through. The desired motion is explicitly progressive:

1. The playing Crystal enlarges first.
2. It moves toward the outgoing side on a shallow turntable/carousel arc.
3. During travel it becomes smaller and gradually disappears.
4. The next Crystal starts almost invisible from the opposite side.
5. It grows, becomes fully visible and only then settles at the playback anchor.

## v0.5.3 motion contract

- Normal duration: 1450 ms.
- Reduced Motion duration: 280 ms stable-anchor cross-dissolve.
- Outgoing scale: `1.0 → 1.11 → 0.34`.
- Incoming scale: `0.34 → 1.04 → 1.0`.
- Audio/metadata switch: 14% of the timeline, while incoming opacity is still
  close to zero.
- Destination content landing begins: 70%.
- Overlay identity fades only in the final 16%, while the real identity and
  controls are already appearing underneath.
- Movement uses a shallow vertical arc and small rotation. It must not resemble
  a 3D cube, cover-flow shelf or game carousel.

## Acceptance criteria

- The outgoing Crystal visibly enlarges before horizontal movement begins.
- The outgoing Crystal continuously shrinks and fades while moving away.
- The incoming Crystal continuously grows and fades in; it must not pop in.
- No black frame, hard replacement or final size snap.
- Final settling is slower than the initial response but contains no fixed
  pause.
- Next moves current identity left and incoming identity from the right;
  Previous mirrors the direction.
- Circular queue behavior from v0.5.1 remains unchanged.
