# AUR-ARTWORK-LOAD-GUARD-FIX-10

## Problem

The glass-shard scene instantiated every cropped image and every mask before
the main processed artwork finished loading. Pack 09 can create hundreds of
image and mask items, which can starve or invalidate the visible artwork load.

## Fix

- load the complete processed artwork first;
- do not gate the visible artwork on a transient service-ready flag;
- arm the shard field only after the main image reaches Image.Ready;
- instantiate shard delegates only while the shard field is actually visible;
- destroy shard delegates immediately when the artwork source changes;
- avoid cached stale OpacityMask textures between tracks.
