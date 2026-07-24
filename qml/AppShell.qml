import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int currentPage: 0 // 0 Home, 1 Music Space, 2 Gallery
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property rect transitionStartRect: Qt.rect(0, 0, 0, 0)
    property rect transitionEndRect: Qt.rect(0, 0, 0, 0)
    property string transitionTitle: "Quiet Signals"
    property string transitionArtist: "Aurora Demo"
    property url transitionArtworkSource:
        "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
    property color transitionIdentityColor: AuroraTokens.coolAccent

    readonly property bool identityTransitionRunning: identityTransition.visible

    function prepareCurrentIdentity() {
        transitionTitle = AudioRuntime.hasTrack ? AudioRuntime.title : "Quiet Signals"
        transitionArtist = AudioRuntime.hasTrack ? AudioRuntime.artist : "Aurora Demo"
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
        transitionStartRect = home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startForward()
        currentPage = 1
    }

    function recallLatestMoment() {
        if (identityTransitionRunning || !Moments.latestAvailable)
            return

        transitionTitle = Moments.latestTitle
        transitionArtist = Moments.latestArtist
        transitionArtworkSource = Moments.latestArtworkSource
        transitionIdentityColor = Moments.latestIdentityColor
        AudioRuntime.setQueue([Moments.latestUrl])

        transitionStartRect = home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startForward()
        currentPage = 1
    }

    function leaveMusicSpace() {
        if (identityTransitionRunning)
            return

        prepareCurrentIdentity()
        transitionStartRect = home.identityAnchorRect()
        transitionEndRect = musicSpace.identityAnchorRect()
        identityTransition.startReverse()
        currentPage = 0
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
        visible: opacity > 0.001 || identityTransitionRunning
        enabled: root.currentPage === 0 && !identityTransitionRunning
        opacity: identityTransitionRunning
                 ? 1.0 - identityTransition.progress
                 : root.currentPage === 0 ? 1.0 : 0.0
        identityVisible: !identityTransitionRunning
        transitioning: identityTransitionRunning
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        onOpenMusicSpace: root.enterMusicSpace()
        onRecallLatestMoment: root.recallLatestMoment()
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
                 ? identityTransition.progress
                 : root.currentPage === 1 ? 1.0 : 0.0
        identityVisible: !identityTransitionRunning
        transitioning: identityTransitionRunning
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
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
}
