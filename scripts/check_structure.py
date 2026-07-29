#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = [
    "AGENTS.md",
    "CMakeLists.txt",
    "CMakePresets.json",
    "README.md", "packaging/linux/aurora.desktop.in", "packaging/linux/aurora.svg",
    "Aurora_项目计划与UI方案_v1.0.md",
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
    "runtime/artwork/ArtworkIllustrationService.h",
    "runtime/artwork/ArtworkIllustrationService.cpp",
    "runtime/cache/MediaCacheService.h",
    "runtime/cache/MediaCacheService.cpp",
    "runtime/audio/LocalTrackIdentity.h",
    "runtime/audio/LocalTrackIdentity.cpp",
    "runtime/library/LocalLibraryRepository.h",
    "runtime/library/LocalLibraryRepository.cpp",
    "runtime/library/LocalLibraryService.h",
    "runtime/library/LocalLibraryService.cpp",
    "runtime/library/LocalLibraryTrackModel.h",
    "runtime/library/LocalLibraryTrackModel.cpp",
    "runtime/library/LocalLibraryWatcher.h",
    "runtime/library/LocalLibraryWatcher.cpp",
    "runtime/lyrics/LyricsService.h",
    "runtime/lyrics/LyricsService.cpp",
    "runtime/memory/MomentRepository.h",
    "runtime/memory/MomentRepository.cpp",
    "runtime/memory/MomentService.h",
    "runtime/memory/MomentService.cpp",
    "qml/Main.qml",
    "qml/AuroraTokens.qml",
    "qml/AuroraI18n.qml",
    "qml/AudioDiagnostics.qml",
    "qml/AuroraFlowScene.qml",
    "qml/MangaBackdrop.qml",
    "qml/MangaLanguageToggle.qml",
    "qml/LiquidTrackTransition.qml",
    "qml/AuroraCore.qml",
    "qml/AuroraMoment.qml",
    "qml/AuroraCrystal.qml",
    "qml/MemoryFlow.qml",
    "qml/MusicIllustrationSpace.qml",
    "qml/ComponentGallery.qml",
    "qml/IdentityTransitionLayer.qml",
    "fixtures/moments.json",
    "fixtures/tracks.json",
    "tests/test_state_mapper.cpp",
    "tests/test_audio_queue.cpp",
    "tests/test_audio_feature_analyzer.cpp",
    "tests/test_local_track_identity.cpp",
    "tests/test_local_library_repository.cpp",
    "tests/test_local_library_watcher.cpp",
    "tests/test_moment_repository.cpp",
    "tests/test_lyrics_service.cpp",
    "docs/engineering/MVP_v0.1.md",
    "docs/engineering/ARCHITECTURE.md",
    "docs/reviews/AUR-024_INSTALLED_RUNTIME_FIRST_RUN_REVIEW.md",
    "docs/tasks/AUR-024_INSTALLED_RUNTIME_FIRST_RUN.md",
    "docs/reviews/AUR-025_MISSING_SOURCE_RELINK_REVIEW.md",
    "docs/tasks/AUR-025_MISSING_SOURCE_RELINK.md",
    "docs/reviews/AUR-026_PERSISTENT_LIBRARY_ROOTS_RESCAN_REVIEW.md",
    "docs/tasks/AUR-026_PERSISTENT_LIBRARY_ROOTS_RESCAN.md",
    "docs/reviews/AUR-027_AUTOMATIC_LIBRARY_REFRESH_REVIEW.md",
    "docs/tasks/AUR-028_FIRST_REAL_MOMENT_LOOP.md",
    "docs/reviews/AUR-028_FIRST_REAL_MOMENT_LOOP_REVIEW.md",
    "docs/tasks/AUR-029_MEMORY_FLOW_MEANING.md",
    "docs/reviews/AUR-029_MEMORY_FLOW_MEANING_REVIEW.md",
    "docs/tasks/AUR-ARTWORK-INTELLIGENCE-CACHE-PACK-01.md",
    "docs/reviews/AUR-ARTWORK-INTELLIGENCE-CACHE-PACK-01_REVIEW.md",
    "docs/tasks/AUR-ILLUSTRATION-PACK-01_MUSIC_ILLUSTRATION_SPACE.md",
    "docs/reviews/AUR-ILLUSTRATION-PACK-01_MUSIC_ILLUSTRATION_SPACE_REVIEW.md",
    "docs/tasks/AUR-DEMO_SPRINT.md",
    "docs/reviews/AUR-DEMO_SPRINT_REVIEW.md",
    "docs/tasks/AUR-027_AUTOMATIC_LIBRARY_REFRESH.md",
]

missing = [path for path in REQUIRED if not (ROOT / path).is_file()]
if missing:
    raise SystemExit("Missing required files:\n" + "\n".join(missing))

for path in ROOT.rglob("*.qml"):
    text = path.read_text(encoding="utf-8")
    if "blurAmount" in text or "glowRadius" in text or "refractionAmount" in text:
        raise SystemExit(f"Raw rendering API leaked into {path.relative_to(ROOT)}")

print(f"Aurora structure valid ({len(REQUIRED)} required files)")
