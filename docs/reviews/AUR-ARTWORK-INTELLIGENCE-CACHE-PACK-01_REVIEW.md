# AUR-ARTWORK-INTELLIGENCE-CACHE-PACK-01 Review

## Root causes corrected

1. The previous illustration received only `artworkAvailable`, not artwork pixels.
2. Default online catalogue entries could keep an empty `artworkUrl` after merge.
3. Lyrics only searched local sidecar LRC files.
4. Remote playback had no persistent media cache.

## Result

Real artwork now becomes the visual source. The artwork service downloads or
reads the cover, caches it, extracts a monochrome edge/halftone illustration,
and exposes focal/contrast/density parameters to QML. Missing artwork uses the
default Aurora templates.

Online lyrics and remote audio are cached under the Qt application cache
location. The media cache limit defaults to 1 GiB.
