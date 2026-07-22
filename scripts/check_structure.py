#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = [
    "AGENTS.md",
    "CMakeLists.txt",
    "CMakePresets.json",
    "README.md",
    "app/main.cpp",
    "runtime/AuroraTypes.h",
    "runtime/AuroraStateMapper.h",
    "runtime/AuroraStateMapper.cpp",
    "runtime/audio/AudioFeatureAnalyzer.h",
    "runtime/audio/AudioFeatureAnalyzer.cpp",
    "runtime/audio/AudioQueue.h",
    "runtime/audio/AudioQueue.cpp",
    "runtime/audio/AudioRuntime.h",
    "runtime/audio/AudioRuntime.cpp",
    "runtime/audio/LocalTrackIdentity.h",
    "runtime/audio/LocalTrackIdentity.cpp",
    "runtime/library/LocalLibraryRepository.h",
    "runtime/library/LocalLibraryRepository.cpp",
    "runtime/library/LocalLibraryService.h",
    "runtime/library/LocalLibraryService.cpp",
    "qml/Main.qml",
    "qml/AuroraTokens.qml",
    "qml/AudioDiagnostics.qml",
    "qml/AuroraFlowScene.qml",
    "qml/LiquidTrackTransition.qml",
    "qml/AuroraCore.qml",
    "qml/AuroraMoment.qml",
    "qml/AuroraCrystal.qml",
    "qml/ComponentGallery.qml",
    "qml/IdentityTransitionLayer.qml",
    "fixtures/moments.json",
    "fixtures/tracks.json",
    "tests/test_state_mapper.cpp",
    "tests/test_audio_queue.cpp",
    "tests/test_audio_feature_analyzer.cpp",
    "tests/test_local_track_identity.cpp",
    "tests/test_local_library_repository.cpp",
    "docs/engineering/MVP_v0.1.md",
    "docs/engineering/ARCHITECTURE.md",
]

missing = [path for path in REQUIRED if not (ROOT / path).is_file()]
if missing:
    raise SystemExit("Missing required files:\n" + "\n".join(missing))

for path in ROOT.rglob("*.qml"):
    text = path.read_text(encoding="utf-8")
    if "blurAmount" in text or "glowRadius" in text or "refractionAmount" in text:
        raise SystemExit(f"Raw rendering API leaked into {path.relative_to(ROOT)}")

print(f"Aurora structure valid ({len(REQUIRED)} required files)")
