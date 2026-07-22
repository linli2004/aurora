import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false

    signal openMusicSpace()
    signal openGallery()

    function identityAnchorRect() {
        const origin = currentMoment.mapToItem(root,
                                               currentMoment.width * 0.30,
                                               currentMoment.height * 0.12)
        const size = currentMoment.width * 0.40
        return Qt.rect(origin.x, origin.y, size, size)
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: AuroraTokens.coolAccent
        secondaryColor: AuroraTokens.warmAccent
        presenceLevel: AudioRuntime.playing ? 0.22 : 0.18
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: Math.max(48, parent.width * 0.065)
        anchors.topMargin: Math.max(42, parent.height * 0.065)
        spacing: 8

        Text {
            text: "AURORA"
            color: AuroraTokens.textSecondary
            font.pixelSize: 13
            font.letterSpacing: 4
            font.weight: Font.DemiBold
        }

        Text {
            text: "One moment first."
            color: AuroraTokens.textPrimary
            font.pixelSize: Math.max(34, root.width * 0.045)
            font.weight: Font.DemiBold
        }

        Text {
            text: "Music, memory and atmosphere — without leaving the desktop behind."
            color: AuroraTokens.textSecondary
            font.pixelSize: 16
        }
    }

    AuroraMoment {
        id: currentMoment
        anchors.centerIn: parent
        sizePreset: root.width < 980 ? 0 : 1
        title: AudioRuntime.hasTrack ? "Current local session" : "The room after midnight"
        periodLabel: AudioRuntime.hasTrack ? "Now" : "Late Summer Night"
        trackTitle: AudioRuntime.hasTrack ? AudioRuntime.title : "Quiet Signals"
        artist: AudioRuntime.hasTrack ? AudioRuntime.artist : "Aurora Demo"
        artworkSource: AudioRuntime.hasTrack
                       ? AudioRuntime.artworkSource
                       : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        emotionColor: AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
                      ? AudioRuntime.identityColor
                      : AuroraTokens.coolAccent
        confirmedUserNote: AudioRuntime.hasTrack ? "Local-first playback." : "A quiet beginning."
        experienceState: root.transitioning
                         ? AuroraTypes.MomentRecalling
                         : AudioRuntime.hasTrack
                           ? AuroraTypes.MomentPresent
                           : AuroraTypes.MomentMeaningful
        identityVisible: root.identityVisible
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        onActivated: root.openMusicSpace()
        onRecallRequested: root.openMusicSpace()
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        radius: 21
        color: Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.10)

        Text {
            anchors.centerIn: parent
            text: "Component Gallery"
            color: AuroraTokens.textSecondary
            font.pixelSize: 13
        }

        TapHandler { onTapped: root.openGallery() }
    }
}
