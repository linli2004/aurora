cmake_minimum_required(VERSION 3.21)

project(AuroraMusicFramework
    VERSION 0.5.3
    DESCRIPTION "Ubuntu-native personal music environment"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick Multimedia Network Sql DBus)

qt_standard_project_setup(REQUIRES 6.8)
include(GNUInstallDirs)

qt_add_executable(aurora
    app/main.cpp
    runtime/AuroraStateMapper.cpp
    runtime/AuroraStateMapper.h
    runtime/AuroraTypes.h
    runtime/audio/AudioFeatureAnalyzer.cpp
    runtime/audio/AudioFeatureAnalyzer.h
    runtime/audio/AudioQueue.cpp
    runtime/audio/AudioQueue.h
    runtime/audio/AudioRuntime.cpp
    runtime/audio/AudioRuntime.h
    runtime/audio/LocalTrackIdentity.cpp
    runtime/audio/LocalTrackIdentity.h
    runtime/library/LocalLibraryRepository.cpp
    runtime/library/LocalLibraryRepository.h
    runtime/library/LocalLibraryService.cpp
    runtime/library/LocalLibraryService.h
    runtime/library/LocalLibraryTrackModel.cpp
    runtime/library/LocalLibraryTrackModel.h
    runtime/library/LocalLibraryWatcher.cpp
    runtime/library/LocalLibraryWatcher.h
    runtime/platform/MprisService.cpp
    runtime/platform/MprisService.h
    runtime/sources/MusicSourceRegistry.cpp
    runtime/sources/MusicSourceRegistry.h
)

target_sources(aurora PRIVATE
    runtime/memory/MomentRepository.cpp
    runtime/memory/MomentRepository.h
    runtime/memory/MomentService.cpp
    runtime/memory/MomentService.h
)

set(AURORA_QML_FILES
    qml/Main.qml
    qml/AppShell.qml
    qml/AuroraTokens.qml
    qml/AuroraI18n.qml
    qml/AtmosphereField.qml
    qml/AuroraFlowScene.qml
    qml/MangaBackdrop.qml
    qml/MangaLanguageToggle.qml
    qml/LiquidTrackTransition.qml
    qml/AudioDiagnostics.qml
    qml/AuroraCore.qml
    qml/AuroraCrystal.qml
    qml/AuroraMoment.qml
    qml/AuroraHome.qml
    qml/MemoryFlow.qml
    qml/MusicSpace.qml
    qml/ComponentGallery.qml
    qml/IdentityTransitionLayer.qml
)

foreach(qml_file IN LISTS AURORA_QML_FILES)
    get_filename_component(qml_name "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_name}")
endforeach()

set_source_files_properties(qml/AuroraTokens.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

set_source_files_properties(qml/AuroraI18n.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

set_source_files_properties(assets/demo-cover-a.png
    PROPERTIES QT_RESOURCE_ALIAS "assets/demo-cover-a.png"
)
set_source_files_properties(assets/demo-cover-b.png
    PROPERTIES QT_RESOURCE_ALIAS "assets/demo-cover-b.png"
)
set_source_files_properties(fixtures/moments.json
    PROPERTIES QT_RESOURCE_ALIAS "fixtures/moments.json"
)
set_source_files_properties(fixtures/tracks.json
    PROPERTIES QT_RESOURCE_ALIAS "fixtures/tracks.json"
)

qt_add_qml_module(aurora
    URI Aurora.App
    VERSION 1.0
    NO_IMPORT_SCAN
    QML_FILES ${AURORA_QML_FILES}
    RESOURCES
        assets/demo-cover-a.png
        assets/demo-cover-b.png
        fixtures/moments.json
        fixtures/tracks.json
)

target_include_directories(aurora PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(aurora PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Network
    Qt6::Sql
    Qt6::DBus
)

set_target_properties(aurora PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)


configure_file(
    packaging/linux/aurora.desktop.in
    "${CMAKE_CURRENT_BINARY_DIR}/aurora.desktop"
    @ONLY
)

install(TARGETS aurora
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/aurora.desktop"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)

install(FILES packaging/linux/aurora.svg
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
    RENAME aurora.svg
)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)

    qt_add_executable(aurora_state_tests
        tests/test_state_mapper.cpp
        runtime/AuroraStateMapper.cpp
        runtime/AuroraStateMapper.h
        runtime/AuroraTypes.h
    )
    target_include_directories(aurora_state_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_state_tests PRIVATE Qt6::Core Qt6::Test)

    add_test(NAME aurora.state_mapper COMMAND aurora_state_tests)

    qt_add_executable(aurora_audio_queue_tests
        tests/test_audio_queue.cpp
        runtime/audio/AudioQueue.cpp
        runtime/audio/AudioQueue.h
    )
    target_include_directories(aurora_audio_queue_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_audio_queue_tests PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME aurora.audio_queue COMMAND aurora_audio_queue_tests)

    qt_add_executable(aurora_local_track_identity_tests
        tests/test_local_track_identity.cpp
        runtime/audio/LocalTrackIdentity.cpp
        runtime/audio/LocalTrackIdentity.h
    )
    target_include_directories(aurora_local_track_identity_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_local_track_identity_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Multimedia
        Qt6::Test
    )
    add_test(NAME aurora.local_track_identity COMMAND aurora_local_track_identity_tests)

    qt_add_executable(aurora_local_library_repository_tests
        tests/test_local_library_repository.cpp
        runtime/audio/LocalTrackIdentity.cpp
        runtime/audio/LocalTrackIdentity.h
        runtime/library/LocalLibraryRepository.cpp
        runtime/library/LocalLibraryRepository.h
    )
    target_include_directories(aurora_local_library_repository_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_local_library_repository_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Multimedia
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME aurora.local_library_repository COMMAND aurora_local_library_repository_tests)

    qt_add_executable(aurora_moment_repository_tests
        tests/test_moment_repository.cpp
        runtime/memory/MomentRepository.cpp
        runtime/memory/MomentRepository.h
    )
    target_include_directories(aurora_moment_repository_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_moment_repository_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME aurora.moment_repository COMMAND aurora_moment_repository_tests)

    qt_add_executable(aurora_local_library_watcher_tests
        tests/test_local_library_watcher.cpp
        runtime/library/LocalLibraryWatcher.cpp
        runtime/library/LocalLibraryWatcher.h
    )
    target_include_directories(aurora_local_library_watcher_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_local_library_watcher_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME aurora.local_library_watcher COMMAND aurora_local_library_watcher_tests)

    qt_add_executable(aurora_audio_feature_tests
        tests/test_audio_feature_analyzer.cpp
        runtime/audio/AudioFeatureAnalyzer.cpp
        runtime/audio/AudioFeatureAnalyzer.h
    )
    target_include_directories(aurora_audio_feature_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_audio_feature_tests PRIVATE
        Qt6::Core
        Qt6::Multimedia
        Qt6::Test
    )
    add_test(NAME aurora.audio_features COMMAND aurora_audio_feature_tests)

    qt_add_executable(aurora_snapshot_smoke
        tests/snapshot_smoke.cpp
        runtime/AuroraStateMapper.cpp
        runtime/AuroraStateMapper.h
        runtime/AuroraTypes.h
        runtime/audio/AudioFeatureAnalyzer.cpp
        runtime/audio/AudioFeatureAnalyzer.h
        runtime/audio/AudioQueue.cpp
        runtime/audio/AudioQueue.h
        runtime/audio/AudioRuntime.cpp
        runtime/audio/AudioRuntime.h
        runtime/audio/LocalTrackIdentity.cpp
        runtime/audio/LocalTrackIdentity.h
        runtime/library/LocalLibraryRepository.cpp
        runtime/library/LocalLibraryRepository.h
        runtime/library/LocalLibraryService.cpp
        runtime/library/LocalLibraryService.h
        runtime/library/LocalLibraryTrackModel.cpp
        runtime/library/LocalLibraryTrackModel.h
        runtime/library/LocalLibraryWatcher.cpp
        runtime/library/LocalLibraryWatcher.h
        runtime/sources/MusicSourceRegistry.cpp
        runtime/sources/MusicSourceRegistry.h
    )
    target_sources(aurora_snapshot_smoke PRIVATE
        runtime/memory/MomentRepository.cpp
        runtime/memory/MomentRepository.h
        runtime/memory/MomentService.cpp
        runtime/memory/MomentService.h
    )

    set_target_properties(aurora_snapshot_smoke PROPERTIES
        QT_QML_MODULE_NO_IMPORT_SCAN TRUE
    )
    target_include_directories(aurora_snapshot_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_snapshot_smoke PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Multimedia
        Qt6::Network
        Qt6::Sql
    )
    qt_add_qml_module(aurora_snapshot_smoke
        URI Aurora.App
        VERSION 1.0
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/snapshot/Aurora/App"
        NO_IMPORT_SCAN
        QML_FILES ${AURORA_QML_FILES}
        RESOURCES
            assets/demo-cover-a.png
            assets/demo-cover-b.png
            fixtures/moments.json
            fixtures/tracks.json
    )

    qt_add_executable(aurora_music_source_registry_tests
        tests/test_music_source_registry.cpp
        runtime/sources/MusicSourceRegistry.cpp
        runtime/sources/MusicSourceRegistry.h
    )
    set_target_properties(aurora_music_source_registry_tests PROPERTIES
        QT_QML_MODULE_NO_IMPORT_SCAN TRUE
    )
    target_include_directories(aurora_music_source_registry_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(aurora_music_source_registry_tests PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Qml
        Qt6::Test
    )
    add_test(NAME aurora.music_source_registry COMMAND aurora_music_source_registry_tests)

    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME aurora.fixtures
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_fixtures.py
        )
        add_test(
            NAME aurora.structure
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_structure.py
        )
    endif()
endif()
