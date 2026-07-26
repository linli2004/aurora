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
    readonly property real transitionPulse:
        Math.sin(Math.max(0.0, Math.min(1.0, progress)) * Math.PI)
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
                                     : 1280
        progressAnimation.start()
    }

    function startReverse() {
        progressAnimation.stop()
        progress = 1.0
        progressAnimation.from = 1.0
        progressAnimation.to = 0.0
        progressAnimation.duration = reducedMotion
                                     ? AuroraTokens.motionSoft
                                     : 980
        progressAnimation.start()
    }

    Rectangle {
        anchors.fill: parent
        color: root.colorSignature
        opacity: root.reducedMotion ? 0.0 : root.transitionPulse * 0.075
    }

    Item {
        anchors.fill: parent
        visible: !root.reducedMotion
        opacity: root.transitionPulse * 0.72

        Rectangle {
            width: parent.width * 0.58
            height: parent.height * 1.35
            x: -width * (0.58 - root.progress * 0.22)
            y: -parent.height * 0.16
            rotation: -12
            color: Qt.rgba(root.colorSignature.r,
                           root.colorSignature.g,
                           root.colorSignature.b,
                           0.055)
        }

        Rectangle {
            width: parent.width * 0.52
            height: parent.height * 1.28
            x: parent.width - width * (0.42 + root.progress * 0.18)
            y: -parent.height * 0.10
            rotation: 11
            color: Qt.rgba(AuroraTokens.memoryAccent.r,
                           AuroraTokens.memoryAccent.g,
                           AuroraTokens.memoryAccent.b,
                           0.038)
        }
    }

    Repeater {
        model: root.reducedMotion || root.qualityMode === AuroraTypes.Eco ? 1 : 2

        delegate: Rectangle {
            required property int index

            x: root.resolvedX
               - root.resolvedSize * (0.10 + index * 0.065)
            y: root.resolvedY
               - root.resolvedSize * (0.10 + index * 0.065)
            width: root.resolvedSize * (1.20 + index * 0.13)
            height: width
            radius: width * (0.22 + index * 0.016)
            color: "transparent"
            border.width: index === 0 ? 2 : 1
            border.color: Qt.rgba(root.colorSignature.r,
                                  root.colorSignature.g,
                                  root.colorSignature.b,
                                  0.18 - index * 0.035
                                  + root.transitionPulse * 0.10)
            opacity: root.reducedMotion
                     ? 0.18
                     : 0.34 + root.transitionPulse * (0.28 - index * 0.04)
            rotation: root.reducedMotion
                      ? 0
                      : (index % 2 === 0 ? -1 : 1)
                        * (7 + index * 5)
                        * root.transitionPulse
            scale: 1.0 + root.transitionPulse * (0.035 + index * 0.012)
        }
    }

    Rectangle {
        id: identityHalo
        x: root.resolvedX - root.resolvedSize * 0.12
        y: root.resolvedY - root.resolvedSize * 0.12
        width: root.resolvedSize * 1.24
        height: width
        radius: width * 0.24
        color: root.colorSignature
        opacity: root.reducedMotion
                 ? 0.035
                 : 0.035 + root.transitionPulse * 0.10
        scale: root.reducedMotion
               ? 1.0
               : 0.96 + root.transitionPulse * 0.10
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
        context: root.progress < 0.46
                 ? AuroraTypes.Moment
                 : AuroraTypes.MusicSpace
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        heroMode: true
        mangaMode: true
        scale: root.reducedMotion
               ? 1.0
               : 0.97 + root.transitionPulse * 0.075
        rotation: root.reducedMotion
                  ? 0
                  : -1.2 + root.progress * 2.4
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: Math.max(42, parent.width * 0.08)
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(360, parent.width * 0.32)
        spacing: 7
        opacity: root.reducedMotion
                 ? 0.0
                 : root.transitionPulse * Math.min(1.0, root.progress * 3.0)
        visible: opacity > 0.01

        Rectangle {
            width: Math.min(128, parent.width * 0.42)
            height: 3
            radius: 2
            color: root.colorSignature
        }

        Text {
            width: parent.width
            text: root.title
            color: AuroraTokens.mangaInk
            font.pixelSize: 32
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.artist
            color: AuroraTokens.mangaMuted
            font.pixelSize: 14
            elide: Text.ElideRight
        }
    }

    NumberAnimation {
        id: progressAnimation
        target: root
        property: "progress"
        easing.type: Easing.InOutCubic
        onFinished: root.transitionFinished(root.progress >= 0.999)
    }
}
