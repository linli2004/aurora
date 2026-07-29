# AUR-ARTWORK-PARTICLE-DYNAMICS-FORWARD-FIX-03

## Stage 1 — Artwork acquisition observer

MusicSpace now observes `AudioRuntime.artworkSource` itself. This fixes the
online metadata timing case where artwork becomes available after source and
track signals have already fired.

## Stage 2 — Bounded asynchronous retry

Artwork acquisition uses a short debounce and at most three retries. The
particle implementation remains active; the fix does not restore the previous
static pipeline.

## Stage 3 — Progressive rendering

The album-derived monochrome illustration remains visible while its particle
map is loading. Particle explosion activates only after a valid particle map is
ready, so an incomplete particle conversion can no longer make the artwork
appear blank.
