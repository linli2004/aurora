# AUR-ARTWORK-LOAD-GUARD-FIX-10 Review

The irregular glass-shard direction is preserved.

Expected sequence:

1. ArtworkIllustrationService supplies the processed local image URL.
2. One main Image loads and displays it.
3. After a short 180 ms guard, shards may be instantiated.
4. A track change disarms and destroys the old shard field before loading the
   next artwork.

This prevents hundreds of shard images from competing with the main artwork
load.
