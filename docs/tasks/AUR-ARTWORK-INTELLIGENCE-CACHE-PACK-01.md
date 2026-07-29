# AUR-ARTWORK-INTELLIGENCE-CACHE-PACK-01

This pack replaces generic track-seeded illustration as the primary visual
when real album artwork exists.

## Stage 1 — Artwork acquisition

- local embedded artwork continues through `AudioRuntime.artworkSource`;
- online catalogue artwork is enriched instead of discarded during merge;
- remote artwork is cached and converted into monochrome line illustration;
- tracks without artwork keep Aurora's default procedural templates.

## Stage 2 — Artwork-derived Music Illustration Space

- the generated line illustration is derived from the actual cover pixels;
- focal point, edge density and contrast drive restrained motion layers;
- the square cover boundary is not rendered;
- no spectrum bars, HUD or high-saturation effects are introduced.

## Stage 3 — Lyrics and media cache

- local tracks still use sidecar `.lrc`;
- resolved NetEase tracks fetch timed lyrics by provider/song ID and cache them;
- remote audio streams immediately and warms a persistent 1 GiB LRU cache;
- cached playback preserves the original queue/source identity.
