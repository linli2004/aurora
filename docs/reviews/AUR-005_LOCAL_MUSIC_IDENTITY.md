# AUR-005 — Local Music Identity v0.2.1

## Goal

Replace filename-only local audio identity with trusted metadata, embedded artwork and a stable semantic color while retaining a clear Generated Identity fallback.

## Implemented

- Resolves title, track artist, album and track number from `QMediaMetaData`.
- Prefers track-level performers over album artist metadata.
- Extracts `CoverArtImage`, falling back to `ThumbnailImage` when necessary.
- Writes embedded artwork to Aurora's local cache and exposes a local file URL to QML.
- Derives a restrained representative identity color from embedded artwork.
- Preserves Unicode filename fallback when metadata is absent.
- Surfaces provenance as `Embedded artwork · Local metadata`, `Local metadata · Generated identity`, or `Filename fallback · Generated identity`.
- Binds the same artwork and identity color across Home, the identity transition and Music Space.
- Adds metadata and artwork resolver unit tests.

## Trust boundary

- Artwork is only shown when it is embedded in the selected local media.
- Missing artwork is never presented as official artwork.
- Generated Identity remains visibly distinguishable from embedded artwork.
- No online lookup, fingerprinting or external metadata service is used.

## Acceptance checks

1. A tagged local track shows metadata title, artist and album.
2. A track with embedded artwork displays the same artwork in Home, Transform and Music Space.
3. A track without artwork keeps Generated Identity and reports fallback provenance.
4. Unicode filenames remain readable when tags are absent.
5. Switching tracks refreshes metadata and artwork without retaining the previous identity.
6. All automated tests pass.
