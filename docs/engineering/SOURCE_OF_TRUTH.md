# Aurora Source-of-Truth Map

## Document Precedence

1. **Initial Development Baseline 0.1** — engineering handoff and current implementation boundaries.
2. **Component Specification Archives** — frozen domain definitions and validation requirements.
3. **Product Definition v0.1** — vision, philosophy and long-term system intent.
4. **Engineering documents in this directory** — implementation choices for the current repository.
5. **Task specs** — narrowly scoped work; they cannot override frozen boundaries.

## Current Interpretation

- The Product Definition describes the final system, not the first executable scope.
- Aurora Core can be implemented first as the runtime-state primitive.
- Aurora Moment is approved as a specification and can be implemented as an MVP, but it is not yet a Production Primitive.
- Aurora Crystal is allowed as a constrained prototype. Identity invariants remain subject to visual review.
- Aurora Presence, real Intelligence, real Memory Association, providers and deep GNOME integration are deferred.

## Conflict Handling

When two documents conflict:

- preserve primitive responsibility boundaries;
- preserve user ownership and provenance requirements;
- prefer the later component specification for component details;
- prefer the Initial Development Baseline for implementation scope;
- record unresolved conflicts under `docs/reviews/` rather than silently choosing.
