import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property color primaryColor: AuroraTokens.coolAccent
    property color secondaryColor: AuroraTokens.memoryAccent
    property real presenceLevel: 0.2
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced

    // Semantic audio inputs. They remain optional so Gallery and static
    // contexts can continue using the same Atmosphere primitive.
    property bool audioReactiveAvailable: false
    property real audioLevel: 0.0
    property real bassEnergy: 0.0
    property real midEnergy: 0.0
    property real highEnergy: 0.0
    property real transientEnergy: 0.0

    readonly property bool ambientEnabled:
        accessibilityMode === AuroraTypes.AccessibilityNormal
        && qualityMode !== AuroraTypes.Eco
    readonly property bool reactiveEnabled:
        ambientEnabled && audioReactiveAvailable
    readonly property real resolvedPresence:
        Math.max(0.0, Math.min(1.0,
            presenceLevel + (reactiveEnabled ? audioLevel * 0.28 : 0.0)))
    readonly property real resolvedBass: reactiveEnabled ? Math.max(0, Math.min(1, bassEnergy)) : 0
    readonly property real resolvedMid: reactiveEnabled ? Math.max(0, Math.min(1, midEnergy)) : 0
    readonly property real resolvedHigh: reactiveEnabled ? Math.max(0, Math.min(1, highEnergy)) : 0
    readonly property real resolvedTransient:
        reactiveEnabled ? Math.max(0, Math.min(1, transientEnergy)) : 0
    readonly property real resolvedLevel:
        reactiveEnabled ? Math.max(0, Math.min(1, audioLevel)) : 0

    property real driftPhase: 0.0

    clip: true

    NumberAnimation on driftPhase {
        from: 0.0
        to: Math.PI * 2.0
        duration: 18000
        loops: Animation.Infinite
        running: root.ambientEnabled
    }

    Rectangle {
        anchors.fill: parent
        color: AuroraTokens.windowBackground
    }

    Rectangle {
        id: fieldA
        width: Math.max(root.width * 0.48, 300)
        height: width
        radius: width / 2
        x: root.width * 0.10 + (root.ambientEnabled ? Math.sin(root.driftPhase) * 8 : 0)
        y: root.height * 0.02 + (root.ambientEnabled ? Math.cos(root.driftPhase * 0.72) * 6 : 0)
        color: root.primaryColor
        opacity: 0.025 + root.resolvedPresence * 0.10 + root.resolvedBass * 0.035
        scale: 1.0 + root.resolvedBass * 0.11 + root.resolvedTransient * 0.035
        transformOrigin: Item.Center
    }

    Rectangle {
        id: fieldB
        width: Math.max(root.width * 0.38, 250)
        height: width
        radius: width / 2
        x: root.width - width * 0.86
           + (root.ambientEnabled ? Math.cos(root.driftPhase * 0.63) * 10 : 0)
        y: root.height * 0.30
           + (root.ambientEnabled ? Math.sin(root.driftPhase * 0.82) * 7 : 0)
        color: Qt.tint(root.secondaryColor,
                       Qt.rgba(root.primaryColor.r,
                               root.primaryColor.g,
                               root.primaryColor.b,
                               0.16 + root.resolvedHigh * 0.16))
        opacity: 0.020 + root.resolvedPresence * 0.075 + root.resolvedMid * 0.042
        scale: 1.0 + root.resolvedMid * 0.085
        rotation: root.ambientEnabled
                  ? Math.sin(root.driftPhase * 0.55) * 2.2 + root.resolvedHigh * 3.0
                  : 0
        transformOrigin: Item.Center
    }

    Rectangle {
        id: fieldC
        width: Math.max(root.width * 0.24, 180)
        height: width
        radius: width / 2
        x: root.width * 0.43
           + (root.ambientEnabled ? Math.sin(root.driftPhase * 0.91) * 7 : 0)
        y: root.height - height * 0.72
        color: Qt.tint(root.primaryColor,
                       Qt.rgba(root.secondaryColor.r,
                               root.secondaryColor.g,
                               root.secondaryColor.b,
                               0.32))
        opacity: 0.012 + root.resolvedPresence * 0.05 + root.resolvedHigh * 0.032
        scale: 0.98 + root.resolvedHigh * 0.12 + root.resolvedTransient * 0.045
        transformOrigin: Item.Center
    }

    // A low-attention wavefront. It appears only when decoded audio buffers
    // are available and never carries track identity or player state.
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) * (0.18 + root.resolvedBass * 0.10)
        height: width
        radius: width / 2
        color: "transparent"
        border.width: root.reactiveEnabled ? 1 : 0
        border.color: Qt.rgba(root.primaryColor.r,
                              root.primaryColor.g,
                              root.primaryColor.b,
                              0.05 + root.resolvedTransient * 0.15)
        opacity: root.reactiveEnabled ? 0.42 + root.resolvedLevel * 0.32 : 0
        scale: 1.0 + root.resolvedTransient * 0.14
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: AuroraTokens.lineQuiet
    }

}
