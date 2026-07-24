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
    property bool sourcePanelExpanded: false
    property bool controlMode: false
    property bool momentFeedbackVisible: false

    readonly property string displayTitle:
        AudioRuntime.hasTrack ? AudioRuntime.title : AuroraI18n.text("demo.track")
    readonly property string displayArtist:
        AudioRuntime.hasTrack ? AudioRuntime.artist : AuroraI18n.text("demo.artist")
    readonly property string displayArtistLine:
        root.displayArtist === "Local audio" || root.displayArtist === "Online source"
        ? ""
        : root.displayArtist
    readonly property string displayAlbum: AudioRuntime.hasTrack ? AudioRuntime.album : ""
    readonly property color displayIdentityColor:
        AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
        ? AudioRuntime.identityColor
        : AuroraTokens.coolAccent
    readonly property real progressRatio:
        AudioRuntime.duration > 0 ? Math.min(1, AudioRuntime.position / AudioRuntime.duration) : 0
    readonly property bool libraryBrowserAvailable: LocalLibrary.sourceCount > 0 || LocalLibrary.searchText.length > 0
    readonly property bool forceControlsVisible: false
    readonly property bool controlsVisible:
        root.forceControlsVisible
        || root.controlMode
        || root.libraryMenuExpanded
        || root.tracksPanelExpanded
        || root.sourcePanelExpanded
    readonly property real controlOpacity: root.controlsVisible ? 1.0 : 0.0
    readonly property bool emptyMusicPromptVisible:
        !AudioRuntime.hasTrack
        && LocalLibrary.trackCount === 0
        && !root.controlsVisible
    readonly property string firstRunLibraryMessage:
        LocalLibrary.defaultMusicDirectoryAvailable
        ? AuroraI18n.text("music.firstRunMusic")
        : AuroraI18n.text("music.firstRunChoose")
    readonly property bool currentMomentKeepable:
        AudioRuntime.hasTrack
        && AudioRuntime.trackId.length > 0
        && AudioRuntime.filePath.length > 0
    readonly property string sourcePanelStatus:
        MusicSources.busy
        ? AuroraI18n.text("music.sourceImporting")
        : MusicSources.resolving
          ? AuroraI18n.text("music.sourceResolving")
        : MusicSources.errorString.length > 0
          ? MusicSources.errorString
          : MusicSources.sourceCount > 0
            ? AuroraI18n.text("music.sourceConnected")
              + " " + MusicSources.sourceCount
              + " · " + MusicSources.sourceNames.join(" / ")
            : AuroraI18n.text("music.sourceEmpty")
    readonly property color mangaButtonFill: AuroraTokens.mangaPanel
    readonly property color mangaButtonBorder: AuroraTokens.mangaInk
    readonly property color mangaText: AuroraTokens.mangaInk
    readonly property color mangaMutedText: AuroraTokens.mangaMuted

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

        if (Moments.keepCurrentMoment(
            AudioRuntime.trackId,
            AudioRuntime.sourceId,
            AudioRuntime.filePath,
            AudioRuntime.title,
            AudioRuntime.artist,
            AudioRuntime.album,
            AudioRuntime.artworkSource,
            root.displayIdentityColor,
            "")) {
            root.momentFeedbackVisible = true
            momentFeedbackTimer.restart()
        }
    }

    function revealControls() {
        root.controlMode = true
        if (!root.libraryMenuExpanded
                && !root.tracksPanelExpanded
                && !root.sourcePanelExpanded)
            controlHideTimer.restart()
    }

    function playSourceInput(append) {
        const text = sourceInput.text.trim()
        if (text.length === 0)
            return

        if (append)
            AudioRuntime.appendSourcesFromText(text)
        else
            AudioRuntime.setQueueFromText(text)

        sourceInput.focus = false
        root.sourcePanelExpanded = false
    }

    function importSourceInput() {
        const text = sourceInput.text.trim()
        if (text.length === 0)
            return

        MusicSources.importFromText(text)
        sourceInput.text = ""
        sourceInput.focus = false
    }

    function resolveSourceInput() {
        const text = sourceInput.text.trim()
        if (text.length === 0)
            MusicSources.resolveDemoTrack()
        else
            MusicSources.resolveFromText(text)

        sourceInput.focus = false
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
    Keys.onSpacePressed: {
        root.revealControls()
        AudioRuntime.togglePlayback()
    }
    Keys.onLeftPressed: event => {
        root.revealControls()
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(-1)
        else
            AudioRuntime.seekRelative(-5000)
        event.accepted = true
    }
    Keys.onRightPressed: event => {
        root.revealControls()
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(1)
        else
            AudioRuntime.seekRelative(5000)
        event.accepted = true
    }
    Keys.onPressed: event => {
        root.revealControls()
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
        if (visible) {
            root.controlMode = root.forceControlsVisible
            forceActiveFocus()
        }
    }

    onLibraryMenuExpandedChanged: {
        if (libraryMenuExpanded)
            root.revealControls()
        else if (!tracksPanelExpanded)
            controlHideTimer.restart()
    }

    onTracksPanelExpandedChanged: {
        if (tracksPanelExpanded)
            root.revealControls()
        else if (!libraryMenuExpanded && !sourcePanelExpanded)
            controlHideTimer.restart()
    }

    onSourcePanelExpandedChanged: {
        if (sourcePanelExpanded)
            root.revealControls()
        else if (!libraryMenuExpanded && !tracksPanelExpanded)
            controlHideTimer.restart()
    }

    Timer {
        id: controlHideTimer
        interval: 4000
        repeat: false
        onTriggered: {
            if (!root.forceControlsVisible
                    && !root.libraryMenuExpanded
                    && !root.tracksPanelExpanded
                    && !root.sourcePanelExpanded)
                root.controlMode = false
        }
    }

    Timer {
        id: momentFeedbackTimer
        interval: 1800
        repeat: false
        onTriggered: root.momentFeedbackVisible = false
    }

    FileDialog {
        id: audioDialog
        title: AuroraI18n.text("music.audioDialog")
        fileMode: FileDialog.OpenFiles
        nameFilters: [
            "Audio files (*.mp3 *.flac *.ogg *.opus *.wav *.m4a *.aac)",
            "Playlists (*.m3u *.m3u8)",
            "All files (*)"
        ]
        onAccepted: AudioRuntime.setQueue(selectedFiles)
    }

    FolderDialog {
        id: libraryDialog
        title: AuroraI18n.text("music.folderDialog")
        onAccepted: LocalLibrary.scanDirectory(selectedFolder)
    }

    Connections {
        target: MusicSources

        function onMusicUrlResolved(url) {
            AudioRuntime.setQueueFromText(url)
            root.sourcePanelExpanded = false
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        z: 10
        onPositionChanged: root.revealControls()
    }

    MangaBackdrop {
        anchors.fill: parent
        accentColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        energy: (AudioRuntime.playing ? 0.36 : AudioRuntime.hasTrack ? 0.22 : 0.15)
                + AudioRuntime.audioLevel * 0.46
                + AudioRuntime.transientEnergy * 0.18
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.50 : AudioRuntime.hasTrack ? 0.34 : 0.22
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        paperMode: true
        opacity: root.flowSceneEnabled ? 0.0 : 0.24

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
        presenceLevel: AudioRuntime.playing ? 0.56 : AudioRuntime.hasTrack ? 0.38 : 0.22
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        paperMode: true
        opacity: root.flowSceneEnabled ? 0.28 : 0.0

        Behavior on opacity {
            NumberAnimation {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    Item {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -34
        width: Math.min(root.width * 0.58, root.height * 0.78)
        height: width * 0.86
        opacity: AudioRuntime.hasTrack ? 0.42 : 0.24
        scale: 1.0 + AudioRuntime.bassEnergy * 0.035 + AudioRuntime.transientEnergy * 0.020

        Rectangle {
            anchors.fill: parent
            radius: Math.min(width, height) * 0.22
            color: Qt.rgba(root.displayIdentityColor.r,
                           root.displayIdentityColor.g,
                           root.displayIdentityColor.b,
                           0.014 + AudioRuntime.audioLevel * 0.018)
            border.width: 3
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.06 + AudioRuntime.highEnergy * 0.06)
            rotation: -8 + AudioRuntime.midEnergy * 3
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.74
            height: parent.height * 0.62
            radius: Math.min(width, height) * 0.24
            color: Qt.rgba(AuroraTokens.memoryAccent.r,
                           AuroraTokens.memoryAccent.g,
                           AuroraTokens.memoryAccent.b,
                           0.012 + AudioRuntime.highEnergy * 0.022)
            border.width: 2
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.05 + AudioRuntime.transientEnergy * 0.07)
            rotation: 11 - AudioRuntime.highEnergy * 4
        }

        Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: 420; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.max(86, parent.height * 0.11)
        width: 178
        height: 46
        visible: root.emptyMusicPromptVisible
        opacity: root.emptyMusicPromptVisible ? 1.0 : 0.0
        radius: 9
        color: AuroraTokens.mangaPanel
        border.width: 3
        border.color: AuroraTokens.mangaInk
        rotation: 1.0
        z: 9

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.connectSource")
            color: AuroraTokens.mangaInk
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler {
            onTapped: {
                root.sourcePanelExpanded = true
                root.revealControls()
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 28
        width: 88
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.back")
            color: root.mangaText
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.closeRequested() }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 436
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.flowSceneEnabled
                      ? root.displayIdentityColor
                      : root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: root.flowSceneEnabled
                  ? AuroraI18n.text("music.sceneFlow")
                  : AuroraI18n.text("music.sceneField")
            color: root.mangaText
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.flowSceneEnabled = !root.flowSceneEnabled }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 300
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.currentMomentKeepable
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.18)
               : AuroraTokens.mangaWash
        border.width: 2
        border.color: root.currentMomentKeepable
                      ? root.displayIdentityColor
                      : Qt.rgba(AuroraTokens.mangaInk.r,
                                AuroraTokens.mangaInk.g,
                                AuroraTokens.mangaInk.b,
                                0.38)

        Text {
            anchors.centerIn: parent
            text: Moments.latestTrackId === AudioRuntime.trackId
                  ? AuroraI18n.text("music.keepAgain")
                  : AuroraI18n.text("music.keepMoment")
            color: root.currentMomentKeepable
                   ? root.mangaText
                   : root.mangaMutedText
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler {
            enabled: root.currentMomentKeepable
            onTapped: root.keepCurrentMoment()
        }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
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
               : root.mangaMutedText
        font.pixelSize: 10
        elide: Text.ElideRight
        visible: text.length > 0 || root.momentFeedbackVisible
        opacity: root.momentFeedbackVisible ? 1.0 : root.controlOpacity

        Behavior on opacity { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        visible: false
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
        id: sourceButton

        anchors.right: parent.right
        anchors.rightMargin: 164
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 112
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.sourcePanelExpanded
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.20)
               : root.mangaButtonFill
        border.width: 2
        border.color: root.sourcePanelExpanded
                      ? root.displayIdentityColor
                      : root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.source")
            color: root.mangaText
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.sourcePanelExpanded = !root.sourcePanelExpanded }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 28
        anchors.top: sourceButton.bottom
        anchors.topMargin: 10
        width: Math.min(430, root.width - 56)
        height: 250
        visible: root.sourcePanelExpanded
        opacity: root.sourcePanelExpanded ? 1.0 : 0.0
        radius: 8
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: AuroraTokens.mangaInk
        z: 20

        Behavior on opacity { NumberAnimation { duration: 240; easing.type: Easing.OutCubic } }

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 9

            Text {
                width: parent.width
                text: AuroraI18n.text("music.sourceHint")
                color: root.mangaText
                font.pixelSize: 12
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Rectangle {
                width: parent.width
                height: 76
                radius: 8
                color: AuroraTokens.mangaPaper
                border.width: 2
                border.color: sourceInput.activeFocus
                              ? root.displayIdentityColor
                              : AuroraTokens.mangaInk

                TextEdit {
                    id: sourceInput

                    anchors.fill: parent
                    anchors.margins: 10
                    color: root.mangaText
                    selectionColor: Qt.rgba(root.displayIdentityColor.r,
                                            root.displayIdentityColor.g,
                                            root.displayIdentityColor.b, 0.42)
                    selectedTextColor: AuroraTokens.mangaInk
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    clip: true

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        text: AuroraI18n.text("music.sourcePlaceholder")
                        color: root.mangaMutedText
                        font.pixelSize: 12
                        visible: sourceInput.text.length === 0
                                 && !sourceInput.activeFocus
                        elide: Text.ElideRight
                    }
                }
            }

            Row {
                width: parent.width
                height: 34
                spacing: 8

                Rectangle {
                    width: 94
                    height: 34
                    radius: 8
                    color: sourceInput.text.trim().length > 0
                           ? Qt.rgba(root.displayIdentityColor.r,
                                     root.displayIdentityColor.g,
                                     root.displayIdentityColor.b, 0.18)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.importSource")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: sourceInput.text.trim().length > 0
                        onTapped: root.importSourceInput()
                    }
                }

                Rectangle {
                    width: 94
                    height: 34
                    radius: 8
                    color: MusicSources.sourceCount > 0 && !MusicSources.resolving
                           ? Qt.rgba(root.displayIdentityColor.r,
                                     root.displayIdentityColor.g,
                                     root.displayIdentityColor.b, 0.22)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.resolveSource")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: MusicSources.sourceCount > 0 && !MusicSources.resolving
                        onTapped: root.resolveSourceInput()
                    }
                }

                Rectangle {
                    width: 94
                    height: 34
                    radius: 8
                    color: sourceInput.text.trim().length > 0
                           ? Qt.rgba(root.displayIdentityColor.r,
                                     root.displayIdentityColor.g,
                                     root.displayIdentityColor.b, 0.22)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.playAddress")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: sourceInput.text.trim().length > 0
                        onTapped: root.playSourceInput(false)
                    }
                }

                Rectangle {
                    width: 94
                    height: 34
                    radius: 8
                    color: sourceInput.text.trim().length > 0
                           ? Qt.rgba(root.displayIdentityColor.r,
                                     root.displayIdentityColor.g,
                                     root.displayIdentityColor.b, 0.18)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.appendAddress")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: sourceInput.text.trim().length > 0
                        onTapped: root.playSourceInput(true)
                    }
                }
            }

            Text {
                width: parent.width
                text: AuroraI18n.text("music.sourceHelp")
                color: root.mangaMutedText
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }

            Rectangle {
                width: parent.width
                height: 46
                radius: 8
                color: AuroraTokens.mangaWash
                border.width: 1
                border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                      AuroraTokens.mangaInk.g,
                                      AuroraTokens.mangaInk.b,
                                      0.38)

                Text {
                    anchors.fill: parent
                    anchors.margins: 9
                    text: root.sourcePanelStatus
                    color: MusicSources.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : root.mangaMutedText
                    font.pixelSize: 11
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: 2
                }
            }
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AudioRuntime.hasTrack
                  ? AuroraI18n.text("music.changeMusic")
                  : AuroraI18n.text("music.chooseMusic")
            color: root.mangaText
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: audioDialog.open() }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
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
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }

        Rectangle {
            id: libraryMenuButton
            width: 150
            height: 42
            radius: 8
            color: root.libraryMenuExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.20)
                   : root.mangaButtonFill
            border.width: 2
            border.color: root.libraryMenuExpanded
                          ? root.displayIdentityColor
                          : root.mangaButtonBorder

            Text {
                anchors.centerIn: parent
                text: LocalLibrary.scanning
                      ? AuroraI18n.text("music.scanning") + " · " + LocalLibrary.scannedFileCount
                      : AuroraI18n.text("music.library") + " · " + LocalLibrary.trackCount
                color: root.mangaText
                font.pixelSize: 13
                font.weight: Font.DemiBold
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
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: AuroraTokens.mangaInk

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 9

                Text {
                    width: parent.width
                    text: LocalLibrary.lastScanStatus.length > 0
                          ? LocalLibrary.lastScanStatus
                            + (LocalLibrary.scanning
                               ? " · " + LocalLibrary.scannedFileCount
                                 + " " + AuroraI18n.text("music.files")
                               : "")
                          : (LocalLibrary.firstRun
                             ? root.firstRunLibraryMessage
                             : LocalLibrary.trackCount
                               + " " + AuroraI18n.text("music.ready"))
                    color: LocalLibrary.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : root.mangaMutedText
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
                               ? AuroraTokens.mangaWash
                               : AuroraTokens.mangaPaper
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.scanning
                                  ? AuroraI18n.text("music.cancelScan")
                                  : (LocalLibrary.firstRun
                                     && LocalLibrary.defaultMusicDirectoryAvailable
                                     ? AuroraI18n.text("music.scanMusic")
                                     : AuroraI18n.text("music.addFolder"))
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
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
                                         root.displayIdentityColor.b, 0.18)
                               : AuroraTokens.mangaWash
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.sourceCount > 0
                                  ? AuroraI18n.text("music.playLibrary")
                                  : AuroraI18n.text("music.chooseFolder")
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
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
                               : root.mangaMutedText
                        font.pixelSize: 10
                        elide: Text.ElideMiddle
                    }

                    Rectangle {
                        width: 84
                        height: 30
                        radius: 15
                        color: LocalLibrary.scanning
                               ? AuroraTokens.mangaWash
                               : Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.16)
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: AuroraI18n.text("music.rescanAll")
                            color: root.mangaText
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
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
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }

        Rectangle {
            id: tracksButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 132
            height: 42
            radius: 8
            color: root.tracksPanelExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.20)
                   : root.mangaButtonFill
            border.width: 2
            border.color: root.tracksPanelExpanded
                          ? root.displayIdentityColor
                          : root.mangaButtonBorder

            Text {
                anchors.centerIn: parent
                text: root.libraryBrowserAvailable
                      ? AuroraI18n.text("music.tracks") + " · " + LocalLibrary.visibleTrackCount
                      : AuroraI18n.text("music.tracks")
                color: root.mangaText
                font.pixelSize: 13
                font.weight: Font.DemiBold
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
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: AuroraTokens.mangaInk

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
                        text: AuroraI18n.text("music.tracks")
                        color: root.mangaText
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
                               : AuroraTokens.mangaWash
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: AuroraI18n.text("music.playList")
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
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
                    color: AuroraTokens.mangaPaper
                    border.width: 2
                    border.color: searchInput.activeFocus
                                  ? root.displayIdentityColor
                                  : AuroraTokens.mangaInk

                    TextInput {
                        id: searchInput
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        text: LocalLibrary.searchText
                        color: root.mangaText
                        selectionColor: Qt.rgba(root.displayIdentityColor.r,
                                                root.displayIdentityColor.g,
                                                root.displayIdentityColor.b, 0.42)
                        selectedTextColor: AuroraTokens.mangaInk
                        font.pixelSize: 12
                        clip: true
                        onTextEdited: LocalLibrary.searchText = text

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: AuroraI18n.text("music.searchLibrary")
                            color: root.mangaMutedText
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
                                         root.displayIdentityColor.b, 0.16)
                               : Qt.rgba(AuroraTokens.mangaInk.r,
                                         AuroraTokens.mangaInk.g,
                                         AuroraTokens.mangaInk.b,
                                         0.045)
                        border.width: 2
                        border.color: AudioRuntime.filePath === filePath
                                      ? root.displayIdentityColor
                                      : Qt.rgba(AuroraTokens.mangaInk.r,
                                                AuroraTokens.mangaInk.g,
                                                AuroraTokens.mangaInk.b,
                                                0.20)

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
                                color: root.mangaText
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: artist.length > 0
                                      ? artist + (album.length > 0 ? " · " + album : "")
                                      : fileName
                                color: root.mangaMutedText
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
                        text: LocalLibrary.searchText.length > 0
                              ? AuroraI18n.text("music.noMatches")
                              : (LocalLibrary.firstRun
                                 ? AuroraI18n.text("music.scanToBegin")
                                 : AuroraI18n.text("music.noTracks"))
                            color: root.mangaMutedText
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    AudioDiagnostics {
        visible: false
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
        spacing: root.controlsVisible ? 20 : 18

        AuroraCrystal {
            id: mainCrystal
            anchors.horizontalCenter: parent.horizontalCenter
            crystalSize: Math.min(root.controlsVisible ? 410 : 500,
                                  Math.max(280, root.height * (root.controlsVisible ? 0.42 : 0.54)))
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
            heroMode: true
            mangaMode: true
            audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
            audioLevel: AudioRuntime.audioLevel
            bassEnergy: AudioRuntime.bassEnergy
            midEnergy: AudioRuntime.midEnergy
            highEnergy: AudioRuntime.highEnergy
            transientEnergy: AudioRuntime.transientEnergy
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
                color: root.mangaText
                opacity: root.controlsVisible ? 1.0 : 0.76
                font.pixelSize: root.controlsVisible ? 28 : 23
                font.weight: Font.DemiBold
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: root.displayArtistLine
                      + (root.displayAlbum.length > 0
                         ? (root.displayArtistLine.length > 0 ? " · " : "") + root.displayAlbum
                         : "")
                      + (root.controlsVisible && AudioRuntime.queueCount > 0
                         && (root.displayArtistLine.length > 0 || root.displayAlbum.length > 0)
                         ? " · " + (AudioRuntime.currentIndex + 1) + "/" + AudioRuntime.queueCount
                         : root.controlsVisible && AudioRuntime.queueCount > 0
                           ? (AudioRuntime.currentIndex + 1) + "/" + AudioRuntime.queueCount
                         : "")
                color: root.mangaMutedText
                opacity: root.controlsVisible ? 1.0 : 0.62
                font.pixelSize: 14
            }

            Text {
                visible: false
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: AudioRuntime.identityProvenance
                color: root.mangaMutedText
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
                color: root.mangaMutedText
                font.pixelSize: 12
                opacity: root.controlOpacity
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
                opacity: root.controlOpacity
                color: Qt.rgba(AuroraTokens.mangaInk.r,
                               AuroraTokens.mangaInk.g,
                               AuroraTokens.mangaInk.b,
                               root.controlsVisible ? 0.16 : 0.070)

                Rectangle {
                    width: parent.width * root.progressRatio
                    height: parent.height
                    radius: parent.radius
                    color: root.displayIdentityColor
                }

                TapHandler {
                    id: progressTap
                    enabled: root.controlsVisible && AudioRuntime.duration > 0
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
                color: root.mangaMutedText
                font.pixelSize: 12
                opacity: root.controlOpacity
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 18

            Rectangle {
                width: 76
                height: 42
                enabled: root.controlsVisible
                opacity: root.controlOpacity
                scale: 0.965 + root.controlOpacity * 0.035
                radius: 8
                color: root.mangaButtonFill
                border.width: 2
                border.color: root.mangaButtonBorder

                Text {
                    anchors.centerIn: parent
                    text: AuroraI18n.text("music.previous")
                    color: root.mangaText
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }

                TapHandler {
                    enabled: !liquidTrackTransition.running
                    onTapped: root.beginTrackTransition(-1)
                }

                Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                Behavior on scale { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
            }

            AuroraCore {
                anchors.verticalCenter: parent.verticalCenter
                diameter: root.controlsVisible ? 90 : 76
                experienceState: root.transitioning || liquidTrackTransition.running
                                 ? AuroraTypes.CoreGathering
                                 : AudioRuntime.coreExperienceState
                context: AuroraTypes.MusicSpace
                presenceLevel: root.controlsVisible
                               ? (AudioRuntime.playing ? 0.72 : AudioRuntime.hasTrack ? 0.38 : 0.24)
                               : (AudioRuntime.playing ? 0.48 : AudioRuntime.hasTrack ? 0.26 : 0.18)
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
                enabled: root.controlsVisible
                opacity: root.controlOpacity
                scale: 0.965 + root.controlOpacity * 0.035
                radius: 8
                color: root.mangaButtonFill
                border.width: 2
                border.color: root.mangaButtonBorder

                Text {
                    anchors.centerIn: parent
                    text: AuroraI18n.text("music.next")
                    color: root.mangaText
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }

                TapHandler {
                    enabled: !liquidTrackTransition.running
                    onTapped: root.beginTrackTransition(1)
                }

                Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                Behavior on scale { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            enabled: root.controlsVisible
            opacity: root.controlOpacity
            scale: 0.965 + root.controlOpacity * 0.035

            Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
            Behavior on scale { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: AuroraI18n.text("music.volume")
                color: root.mangaMutedText
                font.pixelSize: 12
            }

            Rectangle {
                id: volumeTrack
                width: 140
                height: 5
                radius: 3
                color: Qt.rgba(AuroraTokens.mangaInk.r,
                               AuroraTokens.mangaInk.g,
                               AuroraTokens.mangaInk.b,
                               0.16)

                Rectangle {
                    width: parent.width * AudioRuntime.volume
                    height: parent.height
                    radius: parent.radius
                    color: root.displayIdentityColor
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
                color: root.mangaMutedText
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
