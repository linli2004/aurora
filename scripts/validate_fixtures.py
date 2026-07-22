#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def load(name: str) -> dict:
    path = ROOT / "fixtures" / name
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def validate_provenance(value: dict, label: str) -> None:
    required = {
        "source",
        "processingLocation",
        "generatedBy",
        "userConfirmed",
        "lastUpdated",
        "modelVersion",
    }
    missing = required - set(value)
    require(not missing, f"{label} missing provenance keys: {sorted(missing)}")


def main() -> None:
    moments = load("moments.json")
    tracks = load("tracks.json")

    require(moments.get("schemaVersion") == 1, "moments schemaVersion must be 1")
    require(tracks.get("schemaVersion") == 1, "tracks schemaVersion must be 1")
    require(moments.get("moments"), "at least one Moment fixture is required")
    require(tracks.get("tracks"), "at least one Track fixture is required")

    track_ids = {track["id"] for track in tracks["tracks"]}
    for track in tracks["tracks"]:
        validate_provenance(track["provenance"], f"track {track['id']}")
        signature = track.get("identitySignature", {})
        require("artworkAnchor" in signature, f"track {track['id']} needs artworkAnchor")
        require("colorSignature" in signature, f"track {track['id']} needs colorSignature")
        require("opticalGravity" in signature, f"track {track['id']} needs opticalGravity")

    for moment in moments["moments"]:
        require(moment["musicIdentityReference"] in track_ids,
                f"moment {moment['id']} references a missing track")
        require("suggestedMeaning" in moment, f"moment {moment['id']} needs suggestedMeaning")
        require("confirmedMeaning" in moment, f"moment {moment['id']} needs confirmedMeaning")
        validate_provenance(moment["provenance"], f"moment {moment['id']}")

    print("Aurora fixtures valid")


if __name__ == "__main__":
    main()
