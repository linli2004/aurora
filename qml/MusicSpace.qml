import QtQuick
import QtQuick.Dialogs
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    property bool transitionLanding: false
    property bool diagnosticsVisible: false
    property bool flowSceneEnabled: true
    property bool libraryMenuExpanded: false
    property bool tracksPanelExpanded: false

    readonly property string displayTitle: AudioRuntime.hasTrack ? AudioRuntime.title : "Quiet Signals"
    readonly property string displayArtist: AudioRuntime.hasTrack ? AudioRuntime.artist : "Aurora Demo"
    readonly property string displayAlbum: AudioRuntime.hasTrack ? AudioRuntime.album : ""
    readonly property color displayIdentityColor:
        AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
        ? AudioRuntime.identityColor
        : AuroraTokens.coolAccent
    readonly property real progressRatio:
        AudioRuntime.duration > 0 ? Math.min(1, AudioRuntime.position / AudioRuntime.duration) : 0
    readonly property bool libraryBrowserAvailable: LocalLibrary.sourceCount > 0 || LocalLibrary.searchText.length > 0
    readonly property string firstRunLibraryMessage: LocalLibrary.defaultMusicDirectoryAvailable ? "First run · scan Music folder" : "First run · choose a music folder"
    readonly property bool currentMomentKeepable:
        AudioRuntime.hasTrack
        && AudioRuntime.trackId.length > 0
        && AudioRuntime.filePath.length > 0

    signal closeRequested()

    function identityAnchorRect() {
        const origin = mainCrystal.mapToItem(root, 0, 0)
        return Qt.rect(origin.x, origin.y, mainCrystal.width, mainCrystal.height)
    }

    function formatTime(milliseconds) {
        if (!isFinite(milliseconds) || milliseconds < 0)
            return "0:00"
        const totalSeconds = Math.floor(milliseconds / 1000)
        const minutes = Math.floor(totalSeconds / 60)
        const seconds = totalSeconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    function keepCurrentMoment() {
        if (!root.currentMomentKeepable)
            return

        Moments.keepCurrentMoment(
            AudioRuntime.trackId,
            AudioRuntime.sourceId,
            AudioRuntime.filePath,
            AudioRuntime.title,
            AudioRuntime.artist,
            AudioRuntime.album,
            AudioRuntime.artworkSource,
            root.displayIdentityColor,
            "")
    }

    function beginTrackTransition(direction) {
        if (liquidTrackTransition.running || AudioRuntime.queueCount < 2)
            return

        liquidTrackTransition.outgoingTitle = root.displayTitle
        liquidTrackTransition.outgoingArtist = root.displayArtist
        liquidTrackTransition.outgoingArtworkSource = AudioRuntime.hasTrack
                ? AudioRuntime.artworkSource
                : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        liquidTrackTransition.outgoingColor = root.displayIdentityColor
        liquidTrackTransition.targetRect = root.identityAnchorRect()
        root.transitionLanding = false
        liquidTrackTransition.start(direction)
    }

    focus: visible
    Keys.onSpacePressed: AudioRuntime.togglePlayback()
    Keys.onLeftPressed: event => {
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(-1)
        else
            AudioRuntime.seekRelative(-5000)
        event.accepted = true
    }
    Keys.onRightPressed: event => {
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(1)
        else
            AudioRuntime.seekRelative(5000)
        event.accepted = true
    }
    Keys.onPressed: event => {
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_O) {
            audioDialog.open()
            event.accepted = true
            return
        }
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_L) {
            libraryDialog.open()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_K
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            root.keepCurrentMoment()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_D
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            root.diagnosticsVisible = !root.diagnosticsVisible
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_V
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            root.flowSceneEnabled = !root.flowSceneEnabled
            event.accepted = true
        }
    }

    onVisibleChanged: {
        if (visible)
            forceActiveFocus()
    }

    FileDialog {
        id: audioDialog
        title: "Choose local music"
        fileMode: FileDialog.OpenFiles
        nameFilters: [
            "Audio files (*.mp3 *.flac *.ogg *.opus *.wav *.m4a *.aac)",
            "All files (*)"
        ]
        onAccepted: AudioRuntime.setQueue(selectedFiles)
    }

    FolderDialog {
        id: libraryDialog
        title: "Choose music folder"
        onAccepted: LocalLibrary.scanDirectory(selectedFolder)
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.34 : AudioRuntime.hasTrack ? 0.26 : 0.18
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        opacity: root.flowSceneEnabled ? 0.0 : 1.0

        Behavior on opacity {
            NumberAnimation {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    AuroraFlowScene {
        anchors.fill: parent
        primaryColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.38 : AudioRuntime.hasTrack ? 0.28 : 0.18
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        opacity: root.flowSceneEnabled ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 28
        width: 88
        height: 42
        radius: 21
        color: Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.10)

        Text {
            anchors.centerIn: parent
            text: "Back"
            color: AuroraTokens.textSecondary
            font.pixelSize: 14
        }

        TapHandler { onTapped: root.closeRequested() }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 436
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        radius: 21
        color: root.flowSceneEnabled
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.18)
               : Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: root.flowSceneEnabled
                      ? Qt.rgba(root.displayIdentityColor.r,
                                root.displayIdentityColor.g,
                                root.displayIdentityColor.b, 0.42)
                      : Qt.rgba(1, 1, 1, 0.10)

        Text {
            anchors.centerIn: parent
            text: root.flowSceneEnabled ? "Scene: Flow" : "Scene: Field"
            color: AuroraTokens.textSecondary
            font.pixelSize: 12
        }

        TapHandler { onTapped: root.flowSceneEnabled = !root.flowSceneEnabled }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 300
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        radius: 21
        color: root.currentMomentKeepable
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.16)
               : Qt.rgba(1, 1, 1, 0.045)
        border.width: 1
        border.color: root.currentMomentKeepable
                      ? Qt.rgba(root.displayIdentityColor.r,
                                root.displayIdentityColor.g,
                                root.displayIdentityColor.b, 0.36)
                      : Qt.rgba(1, 1, 1, 0.08)

        Text {
            anchors.centerIn: parent
            text: Moments.latestTrackId === AudioRuntime.trackId
                  ? "Keep again"
                  : "Keep moment"
            color: root.currentMomentKeepable
                   ? AuroraTokens.textSecondary
                   : AuroraTokens.textMuted
            font.pixelSize: 12
        }

        TapHandler {
            enabled: root.currentMomentKeepable
            onTapped: root.keepCurrentMoment()
        }
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 300
        anchors.top: parent.top
        anchors.topMargin: 76
        width: 124
        horizontalAlignment: Text.AlignHCenter
        text: Moments.errorString.length > 0
              ? Moments.errorString
              : Moments.lastStatus
        color: Moments.errorString.length > 0
               ? AuroraTokens.warmAccent
               : AuroraTokens.textMuted
        font.pixelSize: 10
        elide: Text.ElideRight
        visible: text.length > 0
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 172
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 116
        height: 42
        radius: 21
        color: root.diagnosticsVisible
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.18)
               : Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: AudioRuntime.audioReactiveAvailable
                      ? Qt.rgba(root.displayIdentityColor.r,
                                root.displayIdentityColor.g,
                                root.displayIdentityColor.b, 0.48)
                      : Qt.rgba(1, 1, 1, 0.10)

        Row {
            anchors.centerIn: parent
            spacing: 7

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 7
                height: 7
                radius: 4
                color: AudioRuntime.audioReactiveAvailable
                       ? (AudioRuntime.playing ? AuroraTokens.warmAccent : AuroraTokens.coolAccent)
                       : AuroraTokens.textMuted
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Audio data"
                color: AuroraTokens.textSecondary
                font.pixelSize: 12
            }
        }

        TapHandler { onTapped: root.diagnosticsVisible = !root.diagnosticsVisible }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        radius: 21
        color: Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.10)

        Text {
            anchors.centerIn: parent
            text: AudioRuntime.hasTrack ? "Change music" : "Choose music"
            color: AuroraTokens.textSecondary
            font.pixelSize: 13
        }

        TapHandler { onTapped: audioDialog.open() }
    }

    Item {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 28
        anchors.topMargin: 82
        width: 246
        height: libraryMenuExpanded
                ? ((LocalLibrary.firstRun || LocalLibrary.libraryRootCount > 0) ? 184 : 150)
                : 42

        Rectangle {
            id: libraryMenuButton
            width: 150
            height: 42
            radius: 21
            color: root.libraryMenuExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.18)
                   : Qt.rgba(1, 1, 1, 0.07)
            border.width: 1
            border.color: root.libraryMenuExpanded
                          ? Qt.rgba(root.displayIdentityColor.r,
                                    root.displayIdentityColor.g,
                                    root.displayIdentityColor.b, 0.42)
                          : Qt.rgba(1, 1, 1, 0.10)

            Text {
                anchors.centerIn: parent
                text: LocalLibrary.scanning
                      ? "Scanning · " + LocalLibrary.scannedFileCount
                      : "Library · " + LocalLibrary.trackCount
                color: AuroraTokens.textSecondary
                font.pixelSize: 13
                elide: Text.ElideRight
            }

            TapHandler {
                onTapped: root.libraryMenuExpanded = !root.libraryMenuExpanded
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: libraryMenuButton.bottom
            anchors.topMargin: 8
            width: parent.width
            height: LocalLibrary.firstRun || LocalLibrary.libraryRootCount > 0 ? 134 : 100
            visible: root.libraryMenuExpanded
            opacity: root.libraryMenuExpanded ? 1 : 0
            radius: 8
            color: Qt.rgba(0.04, 0.05, 0.07, 0.74)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.10)

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 9

                Text {
                    width: parent.width
                    text: LocalLibrary.lastScanStatus.length > 0
                          ? LocalLibrary.lastScanStatus
                            + (LocalLibrary.scanning
                               ? " · " + LocalLibrary.scannedFileCount + " files"
                               : "")
                          : (LocalLibrary.firstRun
                             ? root.firstRunLibraryMessage
                             : LocalLibrary.trackCount + " tracks ready")
                    color: LocalLibrary.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : AuroraTokens.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Row {
                    spacing: 8

                    Rectangle {
                        width: 104
                        height: 30
                        radius: 15
                        color: LocalLibrary.scanning
                               ? Qt.rgba(1, 1, 1, 0.045)
                               : Qt.rgba(1, 1, 1, 0.08)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.10)

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.scanning
                                  ? "Cancel scan"
                                  : (LocalLibrary.firstRun
                                     && LocalLibrary.defaultMusicDirectoryAvailable
                                     ? "Scan Music"
                                     : "Add folder")
                            color: AuroraTokens.textSecondary
                            font.pixelSize: 11
                        }

                        TapHandler {
                            onTapped: {
                                if (LocalLibrary.scanning)
                                    LocalLibrary.cancelScan()
                                else if (LocalLibrary.firstRun
                                         && LocalLibrary.defaultMusicDirectoryAvailable)
                                    LocalLibrary.scanDefaultMusicDirectory()
                                else
                                    libraryDialog.open()
                            }
                        }
                    }

                    Rectangle {
                        width: 104
                        height: 30
                        radius: 15
                        color: LocalLibrary.sourceCount > 0
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.16)
                               : Qt.rgba(1, 1, 1, 0.045)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.10)

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.sourceCount > 0
                                  ? "Play library"
                                  : "Choose folder"
                            color: AuroraTokens.textSecondary
                            font.pixelSize: 11
                        }

                        TapHandler {
                            enabled: LocalLibrary.sourceCount > 0
                                     || !LocalLibrary.scanning
                            onTapped: {
                                if (LocalLibrary.sourceCount > 0)
                                    AudioRuntime.setQueue(LocalLibrary.playableUrls())
                                else
                                    libraryDialog.open()
                            }
                        }
                    }
                }

                Row {
                    width: parent.width
                    height: 30
                    spacing: 8
                    visible: LocalLibrary.libraryRootCount > 0

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 92
                        text: LocalLibrary.libraryRootsSummary
                        color: LocalLibrary.unavailableRootCount > 0
                               ? AuroraTokens.warmAccent
                               : AuroraTokens.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideMiddle
                    }

                    Rectangle {
                        width: 84
                        height: 30
                        radius: 15
                        color: LocalLibrary.scanning
                               ? Qt.rgba(1, 1, 1, 0.045)
                               : Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.14)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.10)

                        Text {
                            anchors.centerIn: parent
                            text: "Rescan all"
                            color: AuroraTokens.textSecondary
                            font.pixelSize: 10
                        }

                        TapHandler {
                            enabled: !LocalLibrary.scanning
                            onTapped: LocalLibrary.rescanLibraryRoots()
                        }
                    }
                }
            }
        }
    }

    Item {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 28
        anchors.bottomMargin: 28
        width: Math.min(336, root.width * 0.31)
        height: root.tracksPanelExpanded ? Math.min(438, root.height - 146) : 42
        visible: root.width >= 920

        Rectangle {
            id: tracksButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 132
            height: 42
            radius: 21
            color: root.tracksPanelExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.18)
                   : Qt.rgba(1, 1, 1, 0.07)
            border.width: 1
            border.color: root.tracksPanelExpanded
                          ? Qt.rgba(root.displayIdentityColor.r,
                                    root.displayIdentityColor.g,
                                    root.displayIdentityColor.b, 0.42)
                          : Qt.rgba(1, 1, 1, 0.10)

            Text {
                anchors.centerIn: parent
                text: root.libraryBrowserAvailable
                      ? "Tracks · " + LocalLibrary.visibleTrackCount
                      : "Tracks"
                color: AuroraTokens.textSecondary
                font.pixelSize: 13
            }

            TapHandler {
                onTapped: root.tracksPanelExpanded = !root.tracksPanelExpanded
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.bottom: tracksButton.top
            anchors.bottomMargin: 10
            width: parent.width
            height: parent.height - tracksButton.height - 10
            visible: root.tracksPanelExpanded
            radius: 8
            color: Qt.rgba(0.04, 0.05, 0.07, 0.74)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.10)

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                Row {
                    width: parent.width
                    height: 28
                    spacing: 8

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 96
                        text: "Tracks"
                        color: AuroraTokens.textSecondary
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        width: 88
                        height: 28
                        radius: 14
                        color: LocalLibrary.visibleTrackCount > 0
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.18)
                               : Qt.rgba(1, 1, 1, 0.045)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.10)

                        Text {
                            anchors.centerIn: parent
                            text: "Play list"
                            color: AuroraTokens.textSecondary
                            font.pixelSize: 11
                        }

                        TapHandler {
                            enabled: LocalLibrary.visibleTrackCount > 0
                            onTapped: AudioRuntime.setQueue(LocalLibrary.visiblePlayableUrls())
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 34
                    radius: 8
                    color: Qt.rgba(1, 1, 1, 0.055)
                    border.width: 1
                    border.color: searchInput.activeFocus
                                  ? Qt.rgba(root.displayIdentityColor.r,
                                            root.displayIdentityColor.g,
                                            root.displayIdentityColor.b, 0.42)
                                  : Qt.rgba(1, 1, 1, 0.10)

                    TextInput {
                        id: searchInput
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        text: LocalLibrary.searchText
                        color: AuroraTokens.textPrimary
                        selectionColor: Qt.rgba(root.displayIdentityColor.r,
                                                root.displayIdentityColor.g,
                                                root.displayIdentityColor.b, 0.42)
                        selectedTextColor: AuroraTokens.textPrimary
                        font.pixelSize: 12
                        clip: true
                        onTextEdited: LocalLibrary.searchText = text

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Search library"
                            color: AuroraTokens.textMuted
                            font.pixelSize: 12
                            visible: searchInput.text.length === 0 && !searchInput.activeFocus
                        }
                    }
                }

                ListView {
                    id: libraryList
                    width: parent.width
                    height: parent.height - 90
                    model: LocalLibrary.tracks
                    clip: true
                    spacing: 6

                    delegate: Rectangle {
                        required property int index
                        required property string title
                        required property string artist
                        required property string album
                        required property string fileName
                        required property string filePath

                        width: libraryList.width
                        height: 58
                        radius: 8
                        color: AudioRuntime.filePath === filePath
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.18)
                               : Qt.rgba(1, 1, 1, 0.045)
                        border.width: 1
                        border.color: AudioRuntime.filePath === filePath
                                      ? Qt.rgba(root.displayIdentityColor.r,
                                                root.displayIdentityColor.g,
                                                root.displayIdentityColor.b, 0.42)
                                      : Qt.rgba(1, 1, 1, 0.08)

                        Column {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 4

                            Text {
                                width: parent.width
                                text: title.length > 0 ? title : fileName
                                color: AuroraTokens.textSecondary
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: artist.length > 0
                                      ? artist + (album.length > 0 ? " · " + album : "")
                                      : fileName
                                color: AuroraTokens.textMuted
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }

                        TapHandler {
                            onTapped: {
                                AudioRuntime.setQueue(LocalLibrary.visiblePlayableUrlsStartingAt(index))
                                root.tracksPanelExpanded = false
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: libraryList.count === 0
                        text: LocalLibrary.searchText.length > 0 ? "No matching tracks" : (LocalLibrary.firstRun ? "Scan your Music folder to begin" : "No tracks")
                            color: AuroraTokens.textMuted
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    AudioDiagnostics {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 28
        anchors.topMargin: 82
        diagnosticsVisible: root.diagnosticsVisible
        reactiveAvailable: AudioRuntime.audioReactiveAvailable
        playbackActive: AudioRuntime.playing
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
    }

    Column {
        id: playerContent
        anchors.centerIn: parent
        opacity: !liquidTrackTransition.running
                 ? 1.0
                 : root.transitionLanding ? 1.0 : 0.02
        scale: !liquidTrackTransition.running
               ? 1.0
               : root.transitionLanding ? 1.0 : 0.972

        Behavior on opacity {
            OpacityAnimator {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : 440
                easing.type: Easing.OutQuint
            }
        }

        Behavior on scale {
            ScaleAnimator {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : 500
                easing.type: Easing.OutQuint
            }
        }
        width: Math.min(parent.width * 0.72, 620)
        spacing: 20

        AuroraCrystal {
            id: mainCrystal
            anchors.horizontalCenter: parent.horizontalCenter
            crystalSize: Math.min(390, Math.max(240, root.height * 0.40))
            title: root.displayTitle
            artist: root.displayArtist
            artworkSource: AudioRuntime.hasTrack
                           ? AudioRuntime.artworkSource
                           : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
            colorSignature: root.displayIdentityColor
            experienceState: root.transitioning
                             ? AuroraTypes.CrystalTransitioning
                             : AuroraTypes.CrystalImmersive
            context: AuroraTypes.MusicSpace
            accessibilityMode: root.accessibilityMode
            qualityMode: root.qualityMode
            opacity: root.identityVisible ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation {
                    duration: AuroraTokens.motionSoft
                    easing.type: Easing.OutCubic
                }
            }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            spacing: 7

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: root.displayTitle
                color: AuroraTokens.textPrimary
                font.pixelSize: 28
                font.weight: Font.DemiBold
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: root.displayArtist
                      + (root.displayAlbum.length > 0 ? " · " + root.displayAlbum : "")
                      + (AudioRuntime.queueCount > 0
                         ? " · " + (AudioRuntime.currentIndex + 1) + "/" + AudioRuntime.queueCount
                         : " · Local-first prototype")
                color: AuroraTokens.textSecondary
                font.pixelSize: 14
            }

            Text {
                visible: AudioRuntime.hasTrack
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: AudioRuntime.identityProvenance
                color: AuroraTokens.textMuted
                font.pixelSize: 11
            }

            Text {
                visible: AudioRuntime.errorString.length > 0
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: AudioRuntime.errorString
                color: AuroraTokens.warmAccent
                font.pixelSize: 13
            }
        }

        Item {
            width: parent.width
            height: 34

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: root.formatTime(AudioRuntime.position)
                color: AuroraTokens.textMuted
                font.pixelSize: 12
            }

            Rectangle {
                id: progressTrack
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 48
                anchors.rightMargin: 48
                anchors.verticalCenter: parent.verticalCenter
                height: 5
                radius: 3
                color: Qt.rgba(1, 1, 1, 0.11)

                Rectangle {
                    width: parent.width * root.progressRatio
                    height: parent.height
                    radius: parent.radius
                    color: AuroraTokens.memoryAccent
                }

                TapHandler {
                    id: progressTap
                    enabled: AudioRuntime.duration > 0
                    onTapped: {
                        AudioRuntime.position = AudioRuntime.duration
                                                * Math.max(0, Math.min(1,
                                                    progressTap.point.position.x / progressTrack.width))
                    }
                }
            }

            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: root.formatTime(AudioRuntime.duration)
                color: AuroraTokens.textMuted
                font.pixelSize: 12
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 18

            Rectangle {
                width: 76
                height: 42
                radius: 21
                color: Qt.rgba(1, 1, 1, 0.07)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.10)

                Text {
                    anchors.centerIn: parent
                    text: "Previous"
                    color: AuroraTokens.textSecondary
                    font.pixelSize: 12
                }

                TapHandler {
                    enabled: !liquidTrackTransition.running
                    onTapped: root.beginTrackTransition(-1)
                }
            }

            AuroraCore {
                anchors.verticalCenter: parent.verticalCenter
                diameter: 90
                experienceState: root.transitioning || liquidTrackTransition.running
                                 ? AuroraTypes.CoreGathering
                                 : AudioRuntime.coreExperienceState
                context: AuroraTypes.MusicSpace
                presenceLevel: AudioRuntime.playing ? 0.72 : AudioRuntime.hasTrack ? 0.38 : 0.24
                accessibilityMode: root.accessibilityMode
                qualityMode: root.qualityMode
                onRequestPlayPause: {
                    if (AudioRuntime.hasTrack)
                        AudioRuntime.togglePlayback()
                    else
                        audioDialog.open()
                }
            }

            Rectangle {
                width: 76
                height: 42
                radius: 21
                color: Qt.rgba(1, 1, 1, 0.07)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.10)

                Text {
                    anchors.centerIn: parent
                    text: "Next"
                    color: AuroraTokens.textSecondary
                    font.pixelSize: 12
                }

                TapHandler {
                    enabled: !liquidTrackTransition.running
                    onTapped: root.beginTrackTransition(1)
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Volume"
                color: AuroraTokens.textMuted
                font.pixelSize: 12
            }

            Rectangle {
                id: volumeTrack
                width: 140
                height: 5
                radius: 3
                color: Qt.rgba(1, 1, 1, 0.11)

                Rectangle {
                    width: parent.width * AudioRuntime.volume
                    height: parent.height
                    radius: parent.radius
                    color: AuroraTokens.coolAccent
                }

                TapHandler {
                    id: volumeTap
                    onTapped: {
                        AudioRuntime.volume = Math.max(0, Math.min(1,
                                                volumeTap.point.position.x / volumeTrack.width))
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Math.round(AudioRuntime.volume * 100) + "%"
                color: AuroraTokens.textMuted
                font.pixelSize: 12
            }
        }
    }

    LiquidTrackTransition {
        id: liquidTrackTransition
        anchors.fill: parent
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        incomingTitle: root.displayTitle
        incomingArtist: root.displayArtist
        incomingArtworkSource: AudioRuntime.hasTrack
                               ? AudioRuntime.artworkSource
                               : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        incomingColor: root.displayIdentityColor

        onSwitchRequested: function(direction) {
            if (direction > 0)
                AudioRuntime.next()
            else
                AudioRuntime.previous()
        }
        onLandingStarted: root.transitionLanding = true
        onCompleted: root.transitionLanding = false
    }

}
