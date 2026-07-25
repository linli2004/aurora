import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    readonly property bool showingLatestMoment:
        Moments.hasMoment
        && (!AudioRuntime.hasTrack
            || Moments.latestTrackId === AudioRuntime.trackId)
    readonly property string homeArtistLine:
        AudioRuntime.artist === "Local audio" ? "" : AudioRuntime.artist

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

    MangaBackdrop {
        anchors.fill: parent
        accentColor: Moments.hasMoment
                     ? Moments.latestIdentityColor
                     : AuroraTokens.coolAccent
        secondaryColor: AuroraTokens.warmAccent
        energy: AudioRuntime.audioLevel * 0.42
                + (Moments.hasMoment ? 0.20 : 0.12)
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
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
        paperMode: true
        opacity: 0.16
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: Math.max(48, parent.width * 0.065)
        anchors.topMargin: Math.max(42, parent.height * 0.065)
        spacing: 8

        Text {
            text: "AURORA"
            color: AuroraTokens.mangaInk
            font.pixelSize: 13
            font.letterSpacing: 4
            font.weight: Font.DemiBold
        }

        Text {
            text: root.showingLatestMoment
                  ? AuroraI18n.momentHeading(Moments.latestHeading)
                  : AudioRuntime.hasTrack
                    ? AuroraI18n.text("home.currentSession")
                  : AuroraI18n.text("home.begin")
            color: AuroraTokens.mangaInk
            font.pixelSize: Math.max(34, root.width * 0.045)
            font.weight: Font.DemiBold
        }

        Rectangle {
            width: Math.min(520, root.width * 0.42)
            height: Math.max(54, homeNote.implicitHeight + 24)
            radius: 8
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: AuroraTokens.mangaInk
            rotation: -1.2

            Text {
                id: homeNote

                anchors.fill: parent
                anchors.margins: 12
                text: root.showingLatestMoment
                      ? (Moments.latestConfirmedMeaning.length > 0
                         ? "「" + Moments.latestConfirmedMeaning + "」"
                         : Moments.latestTitle + " · " + Moments.latestArtist)
                      : AudioRuntime.hasTrack
                        ? AudioRuntime.title + " · " + root.homeArtistLine
                        : AuroraI18n.text("home.beginNote")
                color: AuroraTokens.mangaMuted
                font.pixelSize: 16
                wrapMode: Text.Wrap
            }
        }
    }

    AuroraMoment {
        id: currentMoment
        anchors.centerIn: parent
        sizePreset: root.width < 980 ? 0 : 1
        title: root.showingLatestMoment
               ? AuroraI18n.momentHeading(Moments.latestHeading)
               : AudioRuntime.hasTrack
                 ? AuroraI18n.text("home.currentSession")
                 : AuroraI18n.text("home.latestFallback")
        periodLabel: root.showingLatestMoment
                     ? AuroraI18n.momentPeriod(Moments.latestPeriodLabel)
                     : AudioRuntime.hasTrack
                       ? AuroraI18n.text("home.now")
                       : AuroraI18n.text("home.lateNight")
        trackTitle: root.showingLatestMoment
                    ? Moments.latestTitle
                    : AudioRuntime.hasTrack
                      ? AudioRuntime.title
                      : AuroraI18n.text("demo.track")
        artist: root.showingLatestMoment
                ? Moments.latestArtist
                : AudioRuntime.hasTrack
                  ? root.homeArtistLine
                  : AuroraI18n.text("demo.artist")
        artworkSource: root.showingLatestMoment
                       ? Moments.latestArtworkSource
                       : AudioRuntime.hasTrack
                         ? AudioRuntime.artworkSource
                         : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        emotionColor: root.showingLatestMoment
                      ? Moments.latestIdentityColor
                      : AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
                        ? AudioRuntime.identityColor
                        : AuroraTokens.coolAccent
        confirmedUserNote: root.showingLatestMoment
                           ? Moments.latestConfirmedMeaning
                           : ""
        availability: root.showingLatestMoment && !Moments.latestAvailable
                      ? AuroraTypes.AvailabilityDetached
                      : AuroraTypes.AvailabilityActive
        experienceState: root.transitioning
                         ? AuroraTypes.MomentRecalling
                         : root.showingLatestMoment
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
        mangaMode: true
        onActivated: {
            if (root.showingLatestMoment)
                root.recallLatestMoment()
            else
                root.openMusicSpace()
        }
        onRecallRequested: {
            if (root.showingLatestMoment)
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
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: Moments.hasMoment
                      ? Moments.latestIdentityColor
                      : AuroraTokens.mangaInk

        Text {
            anchors.centerIn: parent
            text: Moments.hasMoment
                  ? AuroraI18n.text("home.memory") + " · " + Moments.momentCount
                  : AuroraI18n.text("home.memory")
            color: AuroraTokens.mangaInk
            font.pixelSize: 13
            font.weight: Font.DemiBold
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
