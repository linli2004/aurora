import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property bool diagnosticsVisible: false
    property bool reactiveAvailable: false
    property bool playbackActive: false
    property real audioLevel: 0.0
    property real bassEnergy: 0.0
    property real midEnergy: 0.0
    property real highEnergy: 0.0
    property real transientEnergy: 0.0

    width: 286
    height: 238
    visible: opacity > 0.001
    opacity: diagnosticsVisible ? 1.0 : 0.0
    scale: diagnosticsVisible ? 1.0 : 0.97
    z: 40

    Behavior on opacity {
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    Behavior on scale {
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    component MeterRow: Item {
        required property string label
        required property real value
        required property color accent

        width: parent ? parent.width : 0
        height: 28

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 72
            text: parent.label
            color: AuroraTokens.textSecondary
            font.pixelSize: 11
        }

        Rectangle {
            id: track
            anchors.left: parent.left
            anchors.leftMargin: 76
            anchors.right: valueLabel.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            height: 5
            radius: 3
            color: Qt.rgba(1, 1, 1, 0.10)

            Rectangle {
                width: parent.width * Math.max(0, Math.min(1, value))
                height: parent.height
                radius: parent.radius
                color: accent

                Behavior on width {
                    NumberAnimation { duration: 70; easing.type: Easing.OutQuad }
                }
            }
        }

        Text {
            id: valueLabel
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 34
            horizontalAlignment: Text.AlignRight
            text: Math.round(Math.max(0, Math.min(1, parent.value)) * 100)
            color: AuroraTokens.textMuted
            font.pixelSize: 10
            font.family: "monospace"
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 24
        color: Qt.rgba(0.055, 0.064, 0.082, 0.96)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.12)

        Column {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 6

            Row {
                width: parent.width
                height: 34
                spacing: 9

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 8
                    height: 8
                    radius: 4
                    color: root.reactiveAvailable
                           ? (root.playbackActive ? AuroraTokens.warmAccent : AuroraTokens.coolAccent)
                           : AuroraTokens.textMuted
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 17
                    spacing: 1

                    Text {
                        text: "Audio diagnostics"
                        color: AuroraTokens.textPrimary
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: root.reactiveAvailable
                              ? (root.playbackActive ? "Decoded buffer · live" : "Decoded buffer · quiet")
                              : "Ambient fallback · no decoded buffer"
                        color: AuroraTokens.textMuted
                        font.pixelSize: 10
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Qt.rgba(1, 1, 1, 0.08)
            }

            MeterRow {
                label: "Level"
                value: root.audioLevel
                accent: AuroraTokens.memoryAccent
            }

            MeterRow {
                label: "Bass"
                value: root.bassEnergy
                accent: AuroraTokens.warmAccent
            }

            MeterRow {
                label: "Mid"
                value: root.midEnergy
                accent: AuroraTokens.coolAccent
            }

            MeterRow {
                label: "High"
                value: root.highEnergy
                accent: Qt.lighter(AuroraTokens.coolAccent, 1.25)
            }

            MeterRow {
                label: "Transient"
                value: root.transientEnergy
                accent: Qt.lighter(AuroraTokens.memoryAccent, 1.25)
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignRight
                text: "D · hide diagnostics"
                color: AuroraTokens.textMuted
                font.pixelSize: 9
            }
        }
    }
}
