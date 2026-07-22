# AUR-011 — Progressive Landing & Geometry Handoff

## Problem observed

The v0.5.1 recording showed a fast transition but an abrupt final lock. Two
implementation details caused the perceived cut:

1. The transition overlay used a fixed 360 px identity while the real Music
   Space crystal had a different responsive size.
2. The overlay was removed before the destination content had cross-faded to
   full presence.

## v0.5.2 decision

- The transition receives the exact destination crystal rectangle.
- The incoming identity lands on that exact position and size.
- Travel ends with a small overshoot and a 24% settle phase.
- Destination content begins appearing during the settle phase.
- The overlay identity fades away only while the real identity is already
  visible underneath.
- No fixed pause is introduced.

## Acceptance criteria

- No last-frame size change.
- No last-frame position change.
- No visible overlay-to-content cut.
- Incoming velocity decays progressively instead of stopping at once.
- Reduced Motion keeps the exact geometry handoff without overshoot.
