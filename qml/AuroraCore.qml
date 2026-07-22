import QtQuick
import Aurora.Runtime 1.0

FocusScope {
    id: root

    property int experienceState: AuroraTypes.CoreIdle
    property int context: AuroraTypes.Home
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property int motionMode: AuroraTypes.MotionNormal
    property real presenceLevel: -1
    property bool memoryState: false
    property int transitionMode: AuroraTypes.TransitionNone
    property int diameter: 96

    signal requestPlayPause()

    readonly property real resolvedPresence:
        presenceLevel >= 0 ? presenceLevel : AuroraTokens.corePresence(experienceState)
    readonly property bool ambientEnabled:
        AuroraStateMapper.ambientMotionAllowed(accessibilityMode, motionMode, qualityMode)
        && (experienceState === AuroraTypes.CorePlaying
            || experienceState === AuroraTypes.CoreGathering
            || experienceState === AuroraTypes.CoreRecall)
    readonly property color semanticAccent: AuroraTokens.coreAccent(experienceState)
    readonly property real semanticScale:
        experienceState === AuroraTypes.CoreTransition ? 1.04
        : experienceState === AuroraTypes.CoreGathering ? 0.98
        : 1.0
    readonly property real innerRatio:
        experienceState === AuroraTypes.CoreIdle ? 0.34
        : experienceState === AuroraTypes.CorePaused ? 0.40
        : experienceState === AuroraTypes.CoreGathering ? 0.34
        : experienceState === AuroraTypes.CoreTransition ? 0.58
        : experienceState === AuroraTypes.CoreRecall ? 0.52
        : 0.48

    width: diameter
    height: diameter
    scale: semanticScale
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: experienceState === AuroraTypes.CorePlaying ? "Pause music" : "Play music"

    Keys.onSpacePressed: requestPlayPause()
    Keys.onReturnPressed: requestPlayPause()
    Keys.onEnterPressed: requestPlayPause()

    Behavior on scale {
        enabled: accessibilityMode !== AuroraTypes.ReducedMotion
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: outerSurface
        anchors.fill: parent
        radius: width / 2
        color: root.accessibilityMode === AuroraTypes.HighContrast
               ? AuroraTokens.surfaceHighContrast
               : AuroraTokens.surfaceRaised
        border.width: root.activeFocus ? 3 : 1
        border.color: root.activeFocus
                      ? AuroraTokens.focusRing
                      : Qt.rgba(1, 1, 1, 0.12 + root.resolvedPresence * 0.10)

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.78
            height: width
            radius: width / 2
            color: "transparent"
            border.width: root.qualityMode === AuroraTypes.Eco ? 1 : 2
            border.color: root.semanticAccent
            opacity: 0.16 + root.resolvedPresence * 0.34
        }

        Rectangle {
            id: semanticLight
            anchors.centerIn: parent
            width: parent.width * (root.innerRatio + root.resolvedPresence * 0.06)
            height: width
            radius: width / 2
            color: root.semanticAccent
            opacity: root.experienceState === AuroraTypes.CorePaused ? 0.28 : 0.62

            Behavior on width {
                enabled: root.accessibilityMode !== AuroraTypes.ReducedMotion
                NumberAnimation { duration: AuroraTokens.motionFlow; easing.type: Easing.OutCubic }
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.18
            height: width
            radius: width / 2
            color: AuroraTokens.textPrimary
            opacity: root.experienceState === AuroraTypes.CoreIdle ? 0.42 : 0.90
        }

        Rectangle {
            visible: root.memoryState || root.experienceState === AuroraTypes.CoreRecall
            width: parent.width * 0.12
            height: width
            radius: width / 2
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: parent.width * 0.08
            anchors.topMargin: parent.height * 0.08
            color: AuroraTokens.memoryAccent
            opacity: 0.86
        }

        SequentialAnimation on scale {
            running: root.ambientEnabled
            loops: Animation.Infinite
            NumberAnimation { to: 1.028; duration: 1900; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1.0; duration: 1900; easing.type: Easing.InOutSine }
        }
    }

    TapHandler {
        onTapped: root.requestPlayPause()
    }
}
