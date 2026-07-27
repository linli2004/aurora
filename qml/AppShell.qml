import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int currentPage: 0 // 0 Home, 1 Music Space, 2 Gallery, 3 Memory
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool presentationMode: false
    property int transitionOriginPage: 0
    property int musicReturnPage: 0
    property rect transitionStartRect: Qt.rect(0, 0, 0, 0)
    property rect transitionEndRect: Qt.rect(0, 0, 0, 0)
    property string transitionTitle: AuroraI18n.text("demo.track")
    property string transitionArtist: AuroraI18n.text("demo.artist")
    property url transitionArtworkSource:
        "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
    property color transitionIdentityColor: AuroraTokens.coolAccent

    readonly property bool identityTransitionRunning: identityTransition.visible

    function prepareCurrentIdentity() {
        transitionTitle = AudioRuntime.hasTrack
                ? AudioRuntime.title
                : AuroraI18n.text("demo.track")
        transitionArtist = AudioRuntime.hasTrack
                ? AudioRuntime.artist
                : AuroraI18n.text("demo.artist")
        transitionArtworkSource = AudioRuntime.hasTrack
                ? AudioRuntime.artworkSource
                : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        transitionIdentityColor =
                AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
                ? AudioRuntime.identityColor
                : AuroraTokens.coolAccent
    }

    function enterMusicSpace() {
        if (identityTransitionRunning)
            return

        prepareCurrentIdentity()
        transitionOriginPage = 0
        musicReturnPage = 0
        transitionStartRect = home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startForward()
        currentPage = 1
    }

    function recallMoment(momentId, originPage) {
        if (identityTransitionRunning
                || !Moments.selectMoment(momentId)
                || !Moments.selectedAvailable) {
            return
        }

        transitionTitle = Moments.selectedTitle
        transitionArtist = Moments.selectedArtist
        transitionArtworkSource = Moments.selectedArtworkSource
        transitionIdentityColor = Moments.selectedIdentityColor
        AudioRuntime.setQueueWithMetadata([{
            url: Moments.selectedUrl,
            title: Moments.selectedTitle,
            artist: Moments.selectedArtist,
            album: Moments.selectedAlbum,
            artworkUrl: Moments.selectedArtworkSource
        }])

        transitionOriginPage = originPage
        musicReturnPage = originPage
        transitionStartRect = originPage === 3
                ? memoryFlow.identityAnchorRect()
                : home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startForward()
        currentPage = 1
    }

    function recallLatestMoment() {
        root.recallMoment(Moments.latestMomentId, 0)
    }

    function leaveMusicSpace() {
        if (identityTransitionRunning)
            return

        prepareCurrentIdentity()
        transitionOriginPage = musicReturnPage
        transitionStartRect = musicReturnPage === 3
                ? memoryFlow.identityAnchorRect()
                : home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startReverse()
        currentPage = musicReturnPage
    }

    Connections {
        target: LocalLibrary
        function onLibraryChanged() {
            Moments.refresh()
        }
    }

    AuroraHome {
        id: home

        anchors.fill: parent
        visible: opacity > 0.001
                 || (identityTransitionRunning
                     && root.transitionOriginPage === 0)
        enabled: root.currentPage === 0 && !identityTransitionRunning
        opacity: identityTransitionRunning
                 && root.transitionOriginPage === 0
                 ? Math.max(0.0,
                            1.0 - identityTransition.progress / 0.44)
                 : root.currentPage === 0 ? 1.0 : 0.0
        identityVisible: !(identityTransitionRunning
                           && root.transitionOriginPage === 0)
        transitioning: identityTransitionRunning
                       && root.transitionOriginPage === 0
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        presentationMode: root.presentationMode
        onOpenMusicSpace: root.enterMusicSpace()
        onRecallLatestMoment: root.recallLatestMoment()
        onOpenMemoryFlow: {
            Moments.selectMoment(Moments.latestMomentId)
            root.currentPage = 3
        }
        onOpenGallery: root.currentPage = 2

        Behavior on opacity {
            enabled: !identityTransitionRunning
            NumberAnimation {
                duration: AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    MusicSpace {
        id: musicSpace

        anchors.fill: parent
        visible: opacity > 0.001 || identityTransitionRunning
        enabled: root.currentPage === 1 && !identityTransitionRunning
        opacity: identityTransitionRunning
                 ? Math.max(0.0,
                            Math.min(1.0,
                                     (identityTransition.progress - 0.46) / 0.54))
                 : root.currentPage === 1 ? 1.0 : 0.0
        identityVisible: !identityTransitionRunning
        transitioning: identityTransitionRunning
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        presentationMode: root.presentationMode
        onCloseRequested: root.leaveMusicSpace()

        Behavior on opacity {
            enabled: !identityTransitionRunning
            NumberAnimation {
                duration: AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    ComponentGallery {
        anchors.fill: parent
        visible: opacity > 0.001
        enabled: root.currentPage === 2
        opacity: root.currentPage === 2 ? 1 : 0
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        onAccessibilityModeChanged: root.accessibilityMode = accessibilityMode
        onQualityModeChanged: root.qualityMode = qualityMode
        onCloseRequested: root.currentPage = 0

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }
    }

    MemoryFlow {
        id: memoryFlow

        anchors.fill: parent
        visible: opacity > 0.001
                 || (identityTransitionRunning
                     && root.transitionOriginPage === 3)
        enabled: root.currentPage === 3 && !identityTransitionRunning
        opacity: identityTransitionRunning
                 && root.transitionOriginPage === 3
                 ? Math.max(0.0,
                            1.0 - identityTransition.progress / 0.44)
                 : root.currentPage === 3 ? 1.0 : 0.0
        identityVisible: !(identityTransitionRunning
                           && root.transitionOriginPage === 3)
        transitioning: identityTransitionRunning
                       && root.transitionOriginPage === 3
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        presentationMode: root.presentationMode
        onCloseRequested: root.currentPage = 0
        onRecallMoment: momentId => root.recallMoment(momentId, 3)
        onManageSourcesRequested: {
            root.musicReturnPage = 3
            root.currentPage = 1
        }

        Behavior on opacity {
            enabled: !identityTransitionRunning
            NumberAnimation {
                duration: AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    IdentityTransitionLayer {
        id: identityTransition

        anchors.fill: parent
        fromRect: root.transitionStartRect
        toRect: root.transitionEndRect
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        title: root.transitionTitle
        artist: root.transitionArtist
        artworkSource: root.transitionArtworkSource
        colorSignature: root.transitionIdentityColor
    }

    Shortcut {
        sequence: "Ctrl+H"
        enabled: !root.identityTransitionRunning
        onActivated: root.currentPage = 0
    }

    Shortcut {
        sequence: "Ctrl+M"
        enabled: Moments.hasMoment && !root.identityTransitionRunning
        onActivated: {
            Moments.selectMoment(Moments.latestMomentId)
            root.currentPage = 3
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        width: 164
        height: 30
        radius: 15
        color: Qt.rgba(AuroraTokens.mangaPaper.r,
                       AuroraTokens.mangaPaper.g,
                       AuroraTokens.mangaPaper.b,
                       0.76)
        border.width: 1
        border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                              AuroraTokens.mangaInk.g,
                              AuroraTokens.mangaInk.b,
                              0.16)
        visible: !root.presentationMode
                 && root.currentPage === 0
                 && !root.identityTransitionRunning
        opacity: visible ? 0.72 : 0.0
        z: 490

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("demo.presentationHint")
            color: AuroraTokens.mangaMuted
            font.pixelSize: 10
            font.letterSpacing: 0.5
        }

        Behavior on opacity {
            NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
        }
    }

    MangaLanguageToggle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 28
        anchors.bottomMargin: 26
        z: 500
        visible: !root.presentationMode
                 && (root.currentPage === 0 || root.currentPage === 3)
        opacity: root.identityTransitionRunning ? 0.0 : 1.0

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }
    }
}
