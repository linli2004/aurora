import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property color primaryColor: AuroraTokens.coolAccent
    property color secondaryColor: AuroraTokens.memoryAccent
    property real presenceLevel: 0.30
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced

    property bool audioReactiveAvailable: false
    property real audioLevel: 0.0
    property real bassEnergy: 0.0
    property real midEnergy: 0.0
    property real highEnergy: 0.0
    property real transientEnergy: 0.0

    readonly property bool motionEnabled:
        accessibilityMode === AuroraTypes.AccessibilityNormal
        && qualityMode !== AuroraTypes.Eco
    readonly property bool reactiveEnabled:
        motionEnabled && audioReactiveAvailable
    readonly property real level:
        reactiveEnabled ? Math.max(0, Math.min(1, audioLevel)) : 0.0
    readonly property real bass:
        reactiveEnabled ? Math.max(0, Math.min(1, bassEnergy)) : 0.0
    readonly property real mid:
        reactiveEnabled ? Math.max(0, Math.min(1, midEnergy)) : 0.0
    readonly property real high:
        reactiveEnabled ? Math.max(0, Math.min(1, highEnergy)) : 0.0
    readonly property real transientEnergyLevel:
        reactiveEnabled ? Math.max(0, Math.min(1, transientEnergy)) : 0.0
    readonly property real presence:
        Math.max(0.0, Math.min(1.0, presenceLevel + level * 0.30))
    readonly property int particleCount:
        qualityMode === AuroraTypes.Eco ? 0 : qualityMode === AuroraTypes.Balanced ? 18 : 28

    property real phase: 0.0

    clip: true

    NumberAnimation on phase {
        from: 0.0
        to: Math.PI * 2.0
        duration: 16000
        loops: Animation.Infinite
        running: root.motionEnabled
    }

    Rectangle {
        anchors.fill: parent
        color: AuroraTokens.windowBackground
    }

    // A broad depth veil keeps the scene quiet around the content while
    // allowing the outer field to feel larger than the player controls.
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(root.primaryColor.r,
                               root.primaryColor.g,
                               root.primaryColor.b,
                               0.018 + root.presence * 0.030)
            }
            GradientStop {
                position: 0.58
                color: Qt.rgba(0.02, 0.025, 0.04, 0.0)
            }
            GradientStop {
                position: 1.0
                color: Qt.rgba(root.secondaryColor.r,
                               root.secondaryColor.g,
                               root.secondaryColor.b,
                               0.012 + root.high * 0.025)
            }
        }
    }

    Item {
        id: flowLayer
        anchors.fill: parent
        transformOrigin: Item.Center
        rotation: root.motionEnabled ? Math.sin(root.phase * 0.34) * 1.5 : 0
        scale: 1.0 + root.bass * 0.018

        Rectangle {
            id: ribbonA
            width: Math.max(root.width * 0.78, 720)
            height: Math.max(root.height * 0.21, 150)
            radius: height / 2
            x: -width * 0.17 + Math.sin(root.phase * 0.62) * 20
            y: root.height * 0.12 + Math.cos(root.phase * 0.48) * 15
            rotation: -19 + root.mid * 7
            color: Qt.rgba(root.primaryColor.r,
                           root.primaryColor.g,
                           root.primaryColor.b,
                           0.050 + root.presence * 0.065 + root.bass * 0.030)
            scale: 1.0 + root.bass * 0.10
            transformOrigin: Item.Center
        }

        Rectangle {
            id: ribbonB
            width: Math.max(root.width * 0.68, 620)
            height: Math.max(root.height * 0.16, 120)
            radius: height / 2
            x: root.width * 0.43 + Math.cos(root.phase * 0.51) * 18
            y: root.height * 0.37 + Math.sin(root.phase * 0.70) * 18
            rotation: 24 - root.high * 8
            color: Qt.rgba(root.secondaryColor.r,
                           root.secondaryColor.g,
                           root.secondaryColor.b,
                           0.035 + root.presence * 0.055 + root.mid * 0.035)
            scale: 1.0 + root.mid * 0.08
            transformOrigin: Item.Center
        }

        Rectangle {
            id: ribbonC
            width: Math.max(root.width * 0.55, 500)
            height: Math.max(root.height * 0.11, 90)
            radius: height / 2
            x: root.width * 0.19 + Math.sin(root.phase * 0.82) * 16
            y: root.height * 0.70 + Math.cos(root.phase * 0.59) * 12
            rotation: -7 + root.high * 11
            color: Qt.rgba(root.primaryColor.r * 0.72 + root.secondaryColor.r * 0.28,
                           root.primaryColor.g * 0.72 + root.secondaryColor.g * 0.28,
                           root.primaryColor.b * 0.72 + root.secondaryColor.b * 0.28,
                           0.022 + root.presence * 0.050 + root.high * 0.040)
            scale: 1.0 + root.high * 0.09 + root.transientEnergyLevel * 0.045
            transformOrigin: Item.Center
        }
    }

    // The gravity bloom reacts mainly to low-frequency weight. It remains
    // behind the identity object and never becomes a conventional spectrum.
    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -22
        width: Math.min(root.width, root.height) * (0.42 + root.bass * 0.12)
        height: width
        radius: width / 2
        color: Qt.rgba(root.primaryColor.r,
                       root.primaryColor.g,
                       root.primaryColor.b,
                       0.018 + root.presence * 0.040 + root.bass * 0.025)
        scale: 1.0 + root.transientEnergyLevel * 0.05
    }

    Repeater {
        model: 3

        delegate: Rectangle {
            required property int index

            anchors.centerIn: parent
            anchors.verticalCenterOffset: -22
            width: Math.min(root.width, root.height) * (0.22 + index * 0.095)
            height: width
            radius: width / 2
            color: "transparent"
            border.width: root.reactiveEnabled ? 1 : 0
            border.color: Qt.rgba(root.primaryColor.r,
                                  root.primaryColor.g,
                                  root.primaryColor.b,
                                  0.025 + root.transientEnergyLevel * (0.08 - index * 0.012))
            opacity: root.reactiveEnabled
                     ? 0.35 + root.level * 0.22 - index * 0.06
                     : 0.0
            scale: 1.0 + root.transientEnergyLevel * (0.13 + index * 0.055)
        }
    }

    // Sparse particles expose high-frequency texture without turning the
    // scene into a waveform or equalizer. Positions are deterministic so
    // visual snapshots remain comparable.
    Repeater {
        model: root.particleCount

        delegate: Rectangle {
            required property int index

            readonly property real seed: index * 1.61803398875
            readonly property real orbitX: root.width * (0.18 + (index % 5) * 0.035)
            readonly property real orbitY: root.height * (0.12 + (index % 7) * 0.018)
            readonly property real speed: 0.36 + (index % 6) * 0.045

            width: 2.0 + (index % 3) + root.high * 2.2
            height: width
            radius: width / 2
            x: root.width * 0.50
               + Math.cos(root.phase * speed + seed) * orbitX
               - width / 2
            y: root.height * 0.50
               + Math.sin(root.phase * (speed + 0.08) + seed * 0.74) * orbitY
               - height / 2
            color: index % 3 === 0 ? root.secondaryColor : root.primaryColor
            opacity: 0.035 + root.presence * 0.10 + root.high * 0.28
            scale: 0.85 + root.high * 0.65 + root.transientEnergyLevel * 0.45
            transformOrigin: Item.Center
        }
    }

    // Quiet center protection ensures that title, identity and controls stay
    // readable even when the outer atmosphere becomes energetic.
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(root.width * 0.52, 650)
        height: Math.min(root.height * 0.70, 620)
        radius: Math.min(width, height) * 0.20
        color: Qt.rgba(AuroraTokens.windowBackground.r,
                       AuroraTokens.windowBackground.g,
                       AuroraTokens.windowBackground.b,
                       0.30 - root.level * 0.06)
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: AuroraTokens.lineQuiet
    }
}
