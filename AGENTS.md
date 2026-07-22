# Aurora AI Engineering Constitution

This file is mandatory reading for every AI coding session.

## 1. Source of Truth

Use the following precedence when documents overlap:

1. `docs/specifications/Aurora_Initial_Development_Baseline_0.1.md`
2. Component specifications under `docs/specifications/`
3. `docs/specifications/Aurora_Product_Definition_v0.1.md`
4. Engineering documents under `docs/engineering/`
5. Current task specification

A lower-priority document must not silently override a higher-priority document. Report conflicts before inventing a resolution.

## 2. Frozen Primitive Boundaries

- Aurora Core = runtime and perceptual state.
- Aurora Moment = memory identity, context and human-confirmed meaning.
- Aurora Crystal = music identity and cross-context recognition.
- Aurora Atmosphere = emotional and spatial environment.

Never move responsibilities across these boundaries merely to simplify implementation.

## 3. Architecture Rules

- QML owns presentation, interaction intent and local visual state.
- C++ owns typed runtime state, resolvers, services and persistence boundaries.
- Domain data is read-only from QML.
- User changes are expressed as signals or commands and resolved by a service.
- Lifecycle State, Experience State and Runtime State remain separate.
- Do not expose raw drawing controls such as `glowRadius`, `blurAmount` or `refractionAmount` through public component APIs.
- Components expose semantic properties such as `experienceState`, `presenceLevel`, `qualityMode` and `accessibilityMode`.
- Meaning suggested by a model must never be presented as user-confirmed meaning.

## 4. Visual Rules

- Recognition before decoration.
- Stable geometry, dynamic expression.
- Ambient behavior must remain low-attention outside explicit transitions.
- Eco mode may remove effects, but must preserve identity, state and readability.
- Reduced Motion must disable persistent drift, scale pulsing and depth travel.
- Aurora Moment must not look like a reward card, gem, trophy or rarity item.

## 5. Task Rules

Before editing:

1. Read this file.
2. Read the task's referenced specification sections.
3. Inspect existing code and tests.
4. State the files that will change.

During editing:

- Modify only files needed for the task.
- Do not perform unrelated refactors.
- Do not add a third-party dependency without an ADR.
- Add or update a fixture for new visual/data behavior.
- Add a test for state mapping or data behavior.
- Add a snapshot case name for visual behavior.

Before completion:

1. Configure and build when Qt is available.
2. Run all available tests.
3. Report changed files.
4. Report commands executed.
5. Report unresolved issues honestly.
6. Provide a screenshot or snapshot path for visual tasks.

## 6. Completion Language

Do not call a component "Production Ready" unless its QML implementation, visual snapshots, accessibility checks and production validation have passed.

`Initial Development Complete` means architecture has been proven in running code. It does not mean Production Primitive.
