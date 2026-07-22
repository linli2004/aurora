# ADR-002 — Local Repository and Identity Persistence

## Status

Proposed for AUR-017 review.

## Date

2026-07-22

## Context

Aurora Local Music Alpha needs durable local library state before scanner,
SQLite repository, persistent queue, recent playback, favorites or Moments can
depend on music identity.

AUR-016 introduced Track Identity Contract v0.1. That contract is intentionally
local and bounded: it is stable across path rename, path move and byte-identical
copies, but metadata rewrites, container remuxing and transcoding may change the
fingerprint. AUR-017 must keep that limitation explicit in the database model so
future identity algorithms can migrate without breaking user-owned data.

## Constraints

- C++ owns repository state, identity resolution, migration and persistence.
- QML reads domain data and sends commands; QML must not write SQLite directly.
- Crystal owns music identity and recognition.
- Moment owns memory identity and confirmed meaning; Moments must not bind only
  to mutable source paths.
- Local-first provenance must record generated identity source, processing
  location and algorithm version.
- No third-party dependency is added by this ADR.

## Decision

Use SQLite for the Local Repository, but split persistent music identity into
`tracks` and `track_sources`.

`tracks` represents a logical music object Aurora can recognize across one or
more local sources. `track_sources` represents a concrete file path or local
provider source that can be available, missing, unsupported or detached.

Persist identity algorithm and version with every resolved track identity.
Missing or unreadable files must use an explicitly provisional identity instead
of silently replacing the primary `trackId`.

## Identity Model

```text
Track
├ track_id
├ identity_state
├ identity_algorithm
├ identity_version
├ canonical_title
├ artist
├ album
├ duration_ms
├ preferred_artwork_id
├ created_at
├ updated_at
└ provenance_json

TrackSource
├ source_id
├ track_id
├ source_kind
├ file_path
├ availability
├ availability_reason
├ file_size
├ modified_time
├ last_verified_at
├ created_at
├ updated_at
└ provenance_json
```

## Identity State

```text
Resolved
Provisional
Unavailable
```

- `Resolved`: content was readable and a versioned local identity was produced.
- `Provisional`: source exists in user intent/history, but content identity is
  not yet confirmed.
- `Unavailable`: previously known source cannot currently be used.

For v0.1 IDs:

```text
track:local-content:v1:<sha256>
track:provisional:v1:<source-hash>
source:local-file:v1:<path-hash>
```

AUR-016 code currently emits `local-track:` and `local-file:` strings. The first
SQLite implementation should normalize those into the repository namespaces
above at write time or update the runtime before persistence starts.

## Algorithm Versioning

Persist:

```text
identity_algorithm = bounded-file-sample
identity_version = 1
```

Future versions may include:

```text
v2: container-aware payload fingerprint
v3: decoded audio fingerprint
```

Do not overwrite old identity values in place without creating alias records.

## Track Aliases

Add a `track_aliases` table when identity migration is implemented:

```text
track_aliases
├ alias_id
├ track_id
├ alias_value
├ algorithm
├ version
├ reason
├ created_at
└ provenance_json
```

Reasons include:

```text
AlgorithmMigration
MetadataRewrite
UserRelink
DuplicateMerge
```

## Schema Draft

```sql
CREATE TABLE schema_migrations (
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL
);

CREATE TABLE tracks (
    track_id TEXT PRIMARY KEY,
    identity_state TEXT NOT NULL,
    identity_algorithm TEXT NOT NULL,
    identity_version INTEGER NOT NULL,
    canonical_title TEXT NOT NULL,
    artist TEXT NOT NULL DEFAULT '',
    album TEXT NOT NULL DEFAULT '',
    duration_ms INTEGER,
    preferred_artwork_id TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    provenance_json TEXT NOT NULL
);

CREATE TABLE track_sources (
    source_id TEXT PRIMARY KEY,
    track_id TEXT,
    source_kind TEXT NOT NULL,
    file_path TEXT NOT NULL,
    availability TEXT NOT NULL,
    availability_reason TEXT NOT NULL,
    file_size INTEGER,
    modified_time TEXT,
    last_verified_at TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    provenance_json TEXT NOT NULL,
    FOREIGN KEY(track_id) REFERENCES tracks(track_id)
);

CREATE INDEX idx_track_sources_track_id ON track_sources(track_id);
CREATE INDEX idx_track_sources_file_path ON track_sources(file_path);
CREATE INDEX idx_tracks_canonical_title ON tracks(canonical_title);
```

Deferred tables for the first repository implementation:

```text
track_metadata
artwork
queue_items
settings
track_aliases
```

They are deferred because AUR-018 only needs scanner persistence and incremental
library reads. They must be designed before favorites, persistent queue, artwork
cache migration or Moment attachment ships.

## Canonical Title

`canonical_title` is a search and grouping hint only. It must never merge tracks
or become primary identity.

Normalization order:

```text
Display Title
→ Unicode NFC normalization
→ trim
→ whitespace collapse
→ case folding
→ canonical_title
```

Preserve punctuation. Do not treat full-width and half-width characters as
equivalent in v0.1.

## Availability

Use:

```text
availability
+
availability_reason
```

Initial values:

```text
availability:
Available
Unavailable
Detached
Deleted

availability_reason:
None
Missing
PermissionDenied
OfflineVolume
UnsupportedFormat
CorruptFile
UserRemoved
```

UI may simplify these states, but repository data must keep the reason.

## Artwork

Store artwork as file-cache references, not SQLite blobs, for Alpha.

```text
artwork
├ artwork_id
├ track_id
├ source_id
├ cache_path
├ content_hash
├ width
├ height
├ created_at
└ provenance_json
```

Rationale: artwork files can be large, cache eviction is easier outside SQLite,
and database backups should not silently become media blob archives.

## Thread Ownership

```text
Scanner Worker
→ Repository Command Queue
→ Repository-owned SQLite connection
→ Incremental C++ model updates
→ QML read-only views
```

Rules:

- QML never owns SQLite connections.
- Scanner does not mutate QML models directly.
- SQLite connection stays on the repository worker thread.
- UI receives incremental typed results through C++ models/signals.

## Delete Semantics

Keep four separate commands:

```text
RemoveSource
ForgetTrack
DeleteFileFromDisk
ArchiveMemory
```

- `RemoveSource`: detach one file/source from a Track.
- `ForgetTrack`: remove library identity and user library metadata after
  confirmation.
- `DeleteFileFromDisk`: destructive filesystem action requiring explicit user
  confirmation.
- `ArchiveMemory`: Moment operation; must not delete Track or Source data.

## Export and Privacy

Export must treat local paths as privacy-sensitive.

Export groups:

```text
Track identity
Track sources
Artwork references
Provenance
Aliases
Moment links
```

Users must be able to exclude source paths from export. Provenance should be
exported with generated identity data so future imports can explain origin and
algorithm version.

## Migration

Repository must store:

```text
schemaVersion
identityVersion
createdAt
updatedAt
lastVerifiedAt
```

Migration entry point:

```text
Repository::open()
→ read schema_migrations
→ run ordered migrations inside transactions
→ verify schema
→ expose models
```

Deleting the database and rebuilding is not an accepted migration strategy once
user-authored data, favorites, play history, Moment links or tags exist.

## Options Considered

### Option A — Single `tracks` Table with File Path

Rejected. It collapses identity and source, making duplicate files, missing
files, relink and Moment durability harder.

### Option B — Track / TrackSource Split

Accepted. It reflects `Track Identity != File Path` and supports many local
sources per track.

### Option C — Delay SQLite Until Scanner Exists

Rejected. Scanner behavior would define persistence accidentally and increase
future migration cost.

## Consequences

- More schema complexity than a single table.
- Clearer handling of duplicates, missing files and relink.
- Enables queue, history and Moment data to bind to Track identity instead of
  a mutable file path.
- Requires explicit identity migration and alias design before upgrading the
  fingerprint algorithm.

## Validation

AUR-017 is complete when this ADR is accepted and AUR-018 has a concrete schema
boundary to implement against.

AUR-018 implementation must add repository tests for:

- schema creation;
- migration entry point;
- Track/TrackSource round trip;
- duplicate source handling;
- missing source handling;
- delete command separation;
- export redaction for local paths.

## Rollback

Before implementation, rollback is documentation-only: mark this ADR Superseded
and replace it with a new ADR. After SQLite implementation begins, schema
changes require migrations rather than deleting user data.

## Related Tasks

- AUR-016 — Stable Track Identity
- AUR-018 — Local Library Scanner
- AUR-020 — Persistent Queue and Session Restore
