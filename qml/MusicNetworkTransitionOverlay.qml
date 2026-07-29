import QtQuick
import Aurora.Runtime 1.0
import "MusicNetworkLayout.js" as NetworkLayout

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property color colorSignature: AuroraTokens.coolAccent
    property string transitionState: "Idle"
    property real panelOpacity: 1.0
    property real panelScale: 1.0
    property real revealProgress: 1.0
    property real networkOpacity: 0.0
    property real networkScale: 0.985
    property real travelProgress: 0.0
    property real arrivedPulse: 0.0
    property var currentTrack: ({})
    property var targetTrack: ({})
    property var graphNodes: []
    property string currentKey: ""
    property string targetKey: ""

    readonly property bool running: transitionState !== "Idle"
    readonly property int defaultTravelDuration:
        accessibilityMode === AuroraTypes.AccessibilityReducedMotion
        ? 3000
        : 4200
    readonly property int revealDuration:
        accessibilityMode === AuroraTypes.AccessibilityReducedMotion
        ? 3000
        : 10000

    signal playbackRequested(var targetTrack)
    signal playbackTimedOut()
    signal transitionFinished()

    function startTransition(fromTrack, toTrack, queueTracks) {
        if (running)
            return false

        currentTrack = fromTrack || ({})
        targetTrack = toTrack || ({})
        currentKey = NetworkLayout.keyForTrack(currentTrack, 0)
        targetKey = NetworkLayout.keyForTrack(targetTrack, 1)
        const layoutWidth = width > 240 ? width : 960
        const layoutHeight = height > 180 ? height : 560
        graphNodes = NetworkLayout.prepareNodes(
                    queueTracks || [],
                    currentTrack,
                    targetTrack,
                    layoutWidth,
                    layoutHeight)

        transitionSequence.restart()
        return true
    }

    function confirmPlaybackStarted() {
        if (transitionState !== "WaitingForAudio")
            return

        audioWaitTimer.stop()
        revealSequence.restart()
    }

    function cancelToCurrent() {
        transitionSequence.stop()
        revealSequence.stop()
        audioWaitTimer.stop()
        transitionState = "Idle"
        panelOpacity = 1.0
        panelScale = 1.0
        revealProgress = 1.0
        networkOpacity = 0.0
        networkScale = 0.985
        travelProgress = 0.0
        arrivedPulse = 0.0
    }

    visible: running || networkOpacity > 0.01
    enabled: visible
    opacity: visible ? 1.0 : 0.0
    clip: true
    z: 80

    Rectangle {
        anchors.fill: parent
        color: AuroraTokens.mangaPanel
        opacity: 0.0
    }

    MusicNetworkTransitionGraph {
        id: graph

        anchors.fill: parent
        anchors.margins: Math.max(18, Math.min(parent.width, parent.height) * 0.055)
        nodes: root.graphNodes
        currentKey: root.currentKey
        targetKey: root.targetKey
        colorSignature: root.colorSignature
        networkOpacity: root.networkOpacity
        travelProgress: root.travelProgress
        arrivedPulse: root.arrivedPulse
        scale: root.networkScale
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height) * (0.14 + root.arrivedPulse * 0.20)
        height: width
        radius: width / 2
        color: Qt.rgba(root.colorSignature.r,
                       root.colorSignature.g,
                       root.colorSignature.b,
                       0.12)
        opacity: root.arrivedPulse * root.networkOpacity
        scale: 0.78 + root.arrivedPulse * 0.44
    }

    Timer {
        id: audioWaitTimer

        interval: 7000
        repeat: false
        onTriggered: {
            root.transitionState = "Failed"
            root.playbackTimedOut()
            root.cancelToCurrent()
        }
    }

    SequentialAnimation {
        id: transitionSequence

        ScriptAction {
            script: {
                root.transitionState = "PanelFadingOut"
                root.panelOpacity = 1.0
                root.panelScale = 1.0
                root.revealProgress = 0.0
                root.networkOpacity = 0.0
                root.networkScale = 0.985
                root.travelProgress = 0.0
                root.arrivedPulse = 0.0
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "panelOpacity"
                from: 1.0
                to: 1.0
                duration: 120
                easing.type: Easing.OutCubic
            }
            ScriptAction { script: root.panelScale = 1.0 }
        }

        ScriptAction { script: root.transitionState = "NetworkAppearing" }

        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "networkOpacity"
                from: 0.0
                to: 1.0
                duration: 360
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: root
                property: "networkScale"
                from: 0.985
                to: 1.0
                duration: 420
                easing.type: Easing.OutCubic
            }
        }

        ScriptAction { script: root.transitionState = "Travelling" }

        NumberAnimation {
            target: root
            property: "travelProgress"
            from: 0.0
            to: 1.0
            duration: root.defaultTravelDuration
            easing.type: Easing.InOutSine
        }

        ScriptAction { script: root.transitionState = "NetworkFadingOut" }

        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "arrivedPulse"
                from: 0.0
                to: 1.0
                duration: 620
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: root
                property: "networkOpacity"
                from: 1.0
                to: 0.12
                duration: 720
                easing.type: Easing.OutQuint
            }
        }

        ScriptAction {
            script: {
                root.transitionState = "RequestingPlayback"
                root.playbackRequested(root.targetTrack)
                root.transitionState = "WaitingForAudio"
                audioWaitTimer.restart()
            }
        }
    }

    SequentialAnimation {
        id: revealSequence

        ScriptAction {
            script: {
                root.transitionState = "PlaybackRevealing"
                root.panelOpacity = 1.0
                root.panelScale = 1.0
                root.revealProgress = 0.0
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "networkOpacity"
                to: 0.0
                duration: 520
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: root
                property: "panelOpacity"
                from: 1.0
                to: 1.0
                duration: root.revealDuration
                easing.type: Easing.OutQuint
            }
            NumberAnimation {
                target: root
                property: "panelScale"
                from: 1.0
                to: 1.0
                duration: root.revealDuration
                easing.type: Easing.OutQuint
            }
            NumberAnimation {
                target: root
                property: "revealProgress"
                from: 0.0
                to: 1.0
                duration: root.revealDuration
                easing.type: Easing.OutCubic
            }
        }

        ScriptAction {
            script: {
                root.transitionState = "Idle"
                root.panelOpacity = 1.0
                root.panelScale = 1.0
                root.revealProgress = 1.0
                root.networkOpacity = 0.0
                root.networkScale = 0.985
                root.travelProgress = 0.0
                root.arrivedPulse = 0.0
                root.transitionFinished()
            }
        }
    }
}
