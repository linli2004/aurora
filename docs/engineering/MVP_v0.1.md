# Aurora Experience Prototype v0.1

## Purpose

Prove the Aurora architecture in running Qt/QML code before implementing real audio intelligence, online providers or desktop integration.

## Product Question

Can Aurora present runtime state, music identity and memory identity as distinct but continuous perceptual objects without becoming a conventional player UI?

## Vertical Slice

```text
Launch
  ↓
Aurora Home
  ↓
Current Moment
  ↓
Open Moment
  ↓
Crystal remains recognizable
  ↓
Core awakens
  ↓
Atmosphere expands
  ↓
Music Space
```

## In Scope

- Qt 6/QML desktop window
- Semantic Foundation tokens
- Typed Context, Accessibility, Quality and state enums
- Resolver stub for Moment lifecycle mapping
- Aurora Core states: Idle, Playing, Paused, Gathering, Transition, Recall
- Aurora Moment prototype states: Present, Remembered, Meaningful, Recalling, Archived, Detached
- Aurora Crystal prototype states: Present, Focused, Immersive, Transitioning, Detached, Unavailable
- Mock fixtures
- Keyboard interaction
- Reduced Motion and High Contrast modes
- Component Gallery
- Home → Music Space prototype flow
- State mapping tests and fixture validation

## Deferred

- Real playback and FFT
- Emotion Graph
- Memory confidence algorithm
- Production persistence
- Provider authentication and streaming
- Final ShaderEffect materials
- GNOME extension and wallpaper integration
- Touchpad gestures

## Acceptance Criteria

1. Repository configures and builds on a Qt 6.4+ development environment.
2. Application starts in Aurora Home.
3. Component Gallery exposes Core, Moment and Crystal prototype states.
4. Space/Enter triggers semantic interaction intents.
5. Reduced Motion disables continuous scale and drift animation.
6. High Contrast retains readable text and visible focus.
7. Lifecycle mapping tests pass.
8. Fixtures contain provenance and version fields.
9. Components do not expose raw blur/glow/refraction properties publicly.
10. No component claims Production Ready as a result of this prototype.

## Evidence Required

- successful build log;
- test log;
- screenshot set named with `Component.State.Context.Accessibility.Quality`;
- known deviation list;
- review of Moment → Crystal → Core → Atmosphere continuity.
