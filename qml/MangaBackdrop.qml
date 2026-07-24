import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property color accentColor: AuroraTokens.coolAccent
    property color secondaryColor: AuroraTokens.memoryAccent
    property real energy: 0.0
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced

    readonly property bool motionEnabled:
        accessibilityMode === AuroraTypes.AccessibilityNormal
        && qualityMode !== AuroraTypes.Eco
    readonly property real resolvedEnergy: Math.max(0, Math.min(1, energy))

    property real phase: 0

    clip: true

    NumberAnimation on phase {
        from: 0
        to: Math.PI * 2
        duration: 14000
        loops: Animation.Infinite
        running: root.motionEnabled
    }

    Rectangle {
        anchors.fill: parent
        color: AuroraTokens.mangaPaper
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(root.accentColor.r,
                               root.accentColor.g,
                               root.accentColor.b,
                               0.10 + root.resolvedEnergy * 0.06)
            }
            GradientStop { position: 0.56; color: Qt.rgba(1, 1, 1, 0.0) }
            GradientStop {
                position: 1.0
                color: Qt.rgba(root.secondaryColor.r,
                               root.secondaryColor.g,
                               root.secondaryColor.b,
                               0.10)
            }
        }
    }

    Repeater {
        model: 96

        delegate: Rectangle {
            required property int index

            readonly property int column: index % 16
            readonly property int row: Math.floor(index / 16)

            width: 2 + (index % 4)
            height: width
            radius: width / 2
            x: root.width * (0.03 + column * 0.060)
            y: root.height * (0.08 + row * 0.12)
            color: AuroraTokens.mangaInk
            opacity: 0.040 + root.resolvedEnergy * 0.040
        }
    }

    Repeater {
        model: 18

        delegate: Rectangle {
            required property int index

            width: root.width * (0.44 + index * 0.032)
            height: 2
            radius: 1
            x: -root.width * 0.06
            y: root.height * (0.14 + index * 0.040)
               + Math.sin(root.phase + index) * 8
            rotation: -18
            color: AuroraTokens.mangaInk
            opacity: 0.060 + root.resolvedEnergy * 0.060
        }
    }

    Repeater {
        model: 12

        delegate: Rectangle {
            required property int index

            width: root.width * (0.34 + index * 0.04)
            height: 2
            radius: 1
            x: root.width * (0.62 - index * 0.015)
            y: root.height * (0.13 + index * 0.056)
            rotation: 21
            color: root.secondaryColor
            opacity: 0.052 + root.resolvedEnergy * 0.040
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 2
        border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                              AuroraTokens.mangaInk.g,
                              AuroraTokens.mangaInk.b,
                              0.22)
    }
}
