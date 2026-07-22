import QtQuick
import Aurora.Runtime 1.0

FocusScope {
    id: root

    property string title: "Unknown Track"
    property string artist: "Unknown Artist"
    property url artworkSource: ""
    property color colorSignature: AuroraTokens.coolAccent
    property int experienceState: AuroraTypes.CrystalPresent
    property int context: AuroraTypes.Home
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property real crystalSize: 180

    signal relinkRequested()

    readonly property bool hasArtwork: artworkSource.toString().length > 0
    readonly property bool detached: experienceState === AuroraTypes.CrystalDetached
    readonly property bool unavailable: experienceState === AuroraTypes.CrystalUnavailable
    readonly property bool transitioning: experienceState === AuroraTypes.CrystalTransitioning
    readonly property real semanticScale:
        experienceState === AuroraTypes.CrystalDormant ? 0.965
        : experienceState === AuroraTypes.CrystalFocused ? 1.025
        : experienceState === AuroraTypes.CrystalTransitioning ? 1.018
        : experienceState === AuroraTypes.CrystalImmersive ? 1.0
        : 0.985
    readonly property real semanticOpacity: AuroraTokens.crystalOpacity(experienceState)
    readonly property bool identityLabelVisible:
        context !== AuroraTypes.Moment || detached || unavailable

    width: crystalSize
    height: crystalSize
    scale: semanticScale
    opacity: semanticOpacity
    rotation: transitioning ? 1.2 : 0
    activeFocusOnTab: detached
    Accessible.role: detached ? Accessible.Button : Accessible.Graphic
    Accessible.name: detached ? "Relink music source for " + title : title + " by " + artist

    Behavior on scale {
        enabled: accessibilityMode !== AuroraTypes.ReducedMotion
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    Behavior on opacity {
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    Behavior on rotation {
        enabled: accessibilityMode !== AuroraTypes.ReducedMotion
        NumberAnimation { duration: AuroraTokens.motionFlow; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: identitySurface
        anchors.fill: parent
        radius: width * 0.10
        color: Qt.darker(root.colorSignature, 1.5)
        border.width: root.activeFocus ? 3 : (root.transitioning ? 2 : 1)
        border.color: root.activeFocus
                      ? AuroraTokens.focusRing
                      : root.transitioning
                        ? Qt.rgba(root.colorSignature.r,
                                  root.colorSignature.g,
                                  root.colorSignature.b,
                                  0.58)
                        : AuroraTokens.lineSubtle
        clip: true

        Image {
            anchors.fill: parent
            source: root.artworkSource
            fillMode: Image.PreserveAspectCrop
            visible: root.hasArtwork && !root.unavailable
            asynchronous: true
        }

        Rectangle {
            anchors.fill: parent
            visible: !root.hasArtwork || root.unavailable
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter(root.colorSignature, 1.20) }
                GradientStop { position: 0.52; color: root.colorSignature }
                GradientStop { position: 1.0; color: Qt.darker(root.colorSignature, 1.8) }
            }

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.56
                height: width
                radius: width * 0.26
                color: Qt.rgba(1, 1, 1, 0.08)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.14)
                rotation: 12
            }

            Text {
                anchors.centerIn: parent
                width: parent.width * 0.68
                text: root.title.length > 0 ? root.title.charAt(0).toUpperCase() : "A"
                color: AuroraTokens.textPrimary
                font.pixelSize: parent.width * 0.30
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Recognition-safe inner contour. It remains even in Eco mode.
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.76
            height: parent.height * 0.76
            radius: parent.radius * 0.76
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, root.qualityMode === AuroraTypes.Eco ? 0.10 : 0.16)
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: root.qualityMode === AuroraTypes.Eco ? 0 : 2
            border.color: Qt.rgba(1, 1, 1, 0.07)
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: root.identityLabelVisible ? parent.height * 0.25 : 0
            visible: height > 0
            color: Qt.rgba(0, 0, 0, root.detached || root.unavailable ? 0.58 : 0.18)

            Text {
                anchors.centerIn: parent
                text: root.detached
                      ? "Source unavailable"
                      : root.unavailable
                        ? "Generated identity"
                        : root.artist
                color: AuroraTokens.textPrimary
                font.pixelSize: Math.max(12, root.width * 0.060)
                elide: Text.ElideRight
                width: parent.width - 24
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle {
            visible: root.transitioning
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 2
            border.color: root.colorSignature
            opacity: 0.36

            SequentialAnimation on opacity {
                running: root.transitioning
                         && root.accessibilityMode !== AuroraTypes.ReducedMotion
                         && root.qualityMode !== AuroraTypes.Eco
                loops: Animation.Infinite
                NumberAnimation { to: 0.62; duration: 900; easing.type: Easing.InOutSine }
                NumberAnimation { to: 0.26; duration: 900; easing.type: Easing.InOutSine }
            }
        }
    }

    Keys.onReturnPressed: if (detached) relinkRequested()
    Keys.onEnterPressed: if (detached) relinkRequested()

    TapHandler {
        enabled: root.detached
        onTapped: root.relinkRequested()
    }
}
