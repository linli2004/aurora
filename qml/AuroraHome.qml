import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false

    signal openMusicSpace()
    signal recallLatestMoment()
    signal openMemoryFlow()
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
            text: Moments.hasMoment
                  ? Moments.latestHeading
                  : "Begin with a song."
            color: AuroraTokens.textPrimary
            font.pixelSize: Math.max(34, root.width * 0.045)
            font.weight: Font.DemiBold
        }

        Text {
            width: Math.min(520, root.width * 0.42)
            text: Moments.hasMoment
                  ? (Moments.latestConfirmedMeaning.length > 0
                     ? "\"" + Moments.latestConfirmedMeaning + "\""
                     : Moments.latestTitle + " · " + Moments.latestArtist)
                  : "Let one local track become the first remembered room."
            color: AuroraTokens.textSecondary
            font.pixelSize: 16
            wrapMode: Text.Wrap
        }
    }

    AuroraMoment {
        id: currentMoment
        anchors.centerIn: parent
        sizePreset: root.width < 980 ? 0 : 1
        title: Moments.hasMoment
               ? Moments.latestHeading
               : AudioRuntime.hasTrack
                 ? "Current local session"
                 : "The room after midnight"
        periodLabel: Moments.hasMoment
                     ? Moments.latestPeriodLabel
                     : AudioRuntime.hasTrack ? "Now" : "Late Summer Night"
        trackTitle: Moments.hasMoment
                    ? Moments.latestTitle
                    : AudioRuntime.hasTrack ? AudioRuntime.title : "Quiet Signals"
        artist: Moments.hasMoment
                ? Moments.latestArtist
                : AudioRuntime.hasTrack ? AudioRuntime.artist : "Aurora Demo"
        artworkSource: Moments.hasMoment
                       ? Moments.latestArtworkSource
                       : AudioRuntime.hasTrack
                         ? AudioRuntime.artworkSource
                         : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        emotionColor: Moments.hasMoment
                      ? Moments.latestIdentityColor
                      : AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
                        ? AudioRuntime.identityColor
                        : AuroraTokens.coolAccent
        confirmedUserNote: Moments.hasMoment
                           ? Moments.latestConfirmedMeaning
                           : ""
        availability: Moments.hasMoment && !Moments.latestAvailable
                      ? AuroraTypes.AvailabilityDetached
                      : AuroraTypes.AvailabilityActive
        experienceState: root.transitioning
                         ? AuroraTypes.MomentRecalling
                         : Moments.hasMoment
                           ? Moments.latestAvailable
                             ? Moments.latestConfirmedMeaning.length > 0
                               ? AuroraTypes.MomentMeaningful
                               : AuroraTypes.MomentRemembered
                             : AuroraTypes.MomentDetached
                           : AudioRuntime.hasTrack
                             ? AuroraTypes.MomentPresent
                             : AuroraTypes.MomentDormant
        identityVisible: root.identityVisible
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        onActivated: {
            if (Moments.hasMoment)
                root.recallLatestMoment()
            else
                root.openMusicSpace()
        }
        onRecallRequested: {
            if (Moments.hasMoment)
                root.recallLatestMoment()
            else
                root.openMusicSpace()
        }
        onRelinkRequested: root.openMusicSpace()
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 172
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 132
        height: 42
        radius: 21
        color: Moments.hasMoment
               ? Qt.rgba(Moments.latestIdentityColor.r,
                         Moments.latestIdentityColor.g,
                         Moments.latestIdentityColor.b, 0.16)
               : Qt.rgba(1, 1, 1, 0.055)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.10)

        Text {
            anchors.centerIn: parent
            text: Moments.hasMoment
                  ? "Memory · " + Moments.momentCount
                  : "Memory"
            color: AuroraTokens.textSecondary
            font.pixelSize: 13
        }

        TapHandler {
            enabled: Moments.hasMoment
            onTapped: root.openMemoryFlow()
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        visible: false
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
