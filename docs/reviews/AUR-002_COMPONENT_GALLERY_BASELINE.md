# AUR-002 — Component Gallery Visual Baseline

## Status

Implementation candidate for Ubuntu runtime validation.

## Evidence from Prototype 0.1

The first executable gallery proved that Foundation, Aurora Core, Aurora Moment and Aurora Crystal can render in one Qt/QML scene. It also exposed four issues:

1. Component states were not named, so visual review had no reliable state reference.
2. The two oversized atmosphere circles competed with the primitives.
3. Core, Moment and Crystal states were technically switchable but insufficiently distinguishable.
4. Accessibility only exposed Reduced Motion and did not provide a repeatable High Contrast / Cognitive Minimal review path.

## Changes in 0.1.1

- Rebuilt Component Gallery as three stable preview panels.
- Added current state names, descriptions, previous/next navigation and counters.
- Added Accessibility mode cycle: Normal, Reduced Motion, High Contrast, Cognitive Minimal.
- Added Eco/Balanced quality toggle and deterministic reset.
- Reduced Atmosphere visual dominance while preserving ambient presence.
- Added restrained semantic state expression to Core, Moment and Crystal.
- Preserved stable geometry: state differences come from presence, trace, opacity and semantic light.
- Updated QML module loading for Qt 6.5+ and enabled QTP0001 through the standard project setup.

## Required Ubuntu Validation

1. Clean configure and build.
2. Run all tests.
3. Open Component Gallery.
4. Capture one screenshot for every Core state.
5. Capture Moment Present, Remembered, Meaningful, Recalling, Archived and Detached.
6. Capture Crystal Present, Immersive, Transitioning, Detached and Unavailable.
7. Repeat representative states under Reduced Motion, High Contrast and Eco.
8. Confirm keyboard Tab traversal reaches all gallery controls.

## Non-goals

- Final shader material.
- Real audio runtime.
- Final artwork identity extraction.
- Production visual snapshot automation.
