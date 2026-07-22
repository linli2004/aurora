# ADR-001: Qt 6 + QML for the Initial Client

## Status

Accepted for Initial Development.

## Decision

Use Qt 6.4+ with Qt Quick/QML and CMake for the Ubuntu-native prototype.

## Rationale

- QML directly models stateful visual components and transitions.
- C++ can expose typed runtime state and services without putting business logic in presentation code.
- The same prototype can evolve into the product rather than being discarded as a web mockup.
- Qt supports Linux desktop deployment and later media/system integration.

## Consequences

- The team must learn QML state, binding and scene-graph behavior.
- Visual prototypes should be tested on integrated GPUs early.
- Web-first prototypes are reference material only; executable interaction prototypes are QML.
