import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property rect fromRect: Qt.rect(0, 0, 0, 0)
    property rect toRect: Qt.rect(0, 0, 0, 0)
    property real progress: 0.0
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property string title: "Quiet Signals"
    property string artist: "Aurora Demo"
    property url artworkSource: "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
    property color colorSignature: AuroraTokens.coolAccent

    signal transitionFinished(bool enteredMusicSpace)

    readonly property bool reducedMotion:
        accessibilityMode === AuroraTypes.ReducedMotion
        || accessibilityMode === AuroraTypes.CognitiveMinimal
    readonly property real resolvedSize:
        fromRect.width + (toRect.width - fromRect.width) * progress
    readonly property real resolvedX:
        fromRect.x + (toRect.x - fromRect.x) * progress
    readonly property real resolvedY:
        fromRect.y + (toRect.y - fromRect.y) * progress

    visible: progressAnimation.running
    enabled: false
    z: 100

    function startForward() {
        progressAnimation.stop()
        progress = 0.0
        progressAnimation.from = 0.0
        progressAnimation.to = 1.0
        progressAnimation.duration = reducedMotion
                                     ? AuroraTokens.motionSoft
                                     : AuroraTokens.motionPresence
        progressAnimation.start()
    }

    function startReverse() {
        progressAnimation.stop()
        progress = 1.0
        progressAnimation.from = 1.0
        progressAnimation.to = 0.0
        progressAnimation.duration = reducedMotion
                                     ? AuroraTokens.motionSoft
                                     : AuroraTokens.motionFlow
        progressAnimation.start()
    }

    Rectangle {
        id: identityHalo
        x: root.resolvedX - root.resolvedSize * 0.10
        y: root.resolvedY - root.resolvedSize * 0.10
        width: root.resolvedSize * 1.20
        height: width
        radius: width * 0.18
        color: root.colorSignature
        opacity: root.reducedMotion
                 ? 0.04
                 : 0.04 + Math.sin(root.progress * Math.PI) * 0.10
        scale: root.reducedMotion
               ? 1.0
               : 0.96 + Math.sin(root.progress * Math.PI) * 0.08
    }

    AuroraCrystal {
        id: transitionCrystal
        x: root.resolvedX
        y: root.resolvedY
        crystalSize: root.resolvedSize
        title: root.title
        artist: root.artist
        artworkSource: root.artworkSource
        colorSignature: root.colorSignature
        experienceState: AuroraTypes.CrystalTransitioning
        context: root.progress < 0.5 ? AuroraTypes.Moment : AuroraTypes.MusicSpace
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        heroMode: root.progress >= 0.42
    }

    NumberAnimation {
        id: progressAnimation
        target: root
        property: "progress"
        easing.type: Easing.InOutCubic
        onFinished: root.transitionFinished(root.progress >= 0.999)
    }
}
