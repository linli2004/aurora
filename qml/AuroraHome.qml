import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    property real entryPhase: 0.0

    readonly property bool showingLatestMoment:
        Moments.hasMoment
        && (!AudioRuntime.hasTrack
            || Moments.latestTrackId === AudioRuntime.trackId)
    readonly property string homeTitle:
        root.showingLatestMoment
        ? Moments.latestTitle
        : AudioRuntime.hasTrack
          ? AudioRuntime.title
          : AuroraI18n.text("demo.track")
    readonly property string homeArtist:
        root.showingLatestMoment
        ? Moments.latestArtist
        : AudioRuntime.hasTrack
          ? (AudioRuntime.artist === "Local audio"
             || AudioRuntime.artist === "Online source"
             ? ""
             : AudioRuntime.artist)
          : AuroraI18n.text("demo.artist")
    readonly property string homePeriod:
        root.showingLatestMoment
        ? AuroraI18n.momentPeriod(Moments.latestPeriodLabel)
        : AudioRuntime.hasTrack
          ? AuroraI18n.text("home.now")
          : AuroraI18n.text("home.lateNight")
    readonly property string homeHeading:
        root.showingLatestMoment
        ? AuroraI18n.momentHeading(Moments.latestHeading)
        : AudioRuntime.hasTrack
          ? AuroraI18n.text("home.currentSession")
          : AuroraI18n.text("home.begin")
    readonly property string homeMeaning:
        root.showingLatestMoment ? Moments.latestConfirmedMeaning : ""
    readonly property color homeIdentityColor:
        root.showingLatestMoment
        ? Moments.latestIdentityColor
        : AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
          ? AudioRuntime.identityColor
          : AuroraTokens.coolAccent

    signal openMusicSpace()
    signal recallLatestMoment()
    signal openMemoryFlow()
    signal openGallery()

    function identityAnchorRect() {
        const origin = currentMoment.mapToItem(root,
                                               currentMoment.width * 0.14,
                                               currentMoment.height * 0.10)
        const size = currentMoment.width * 0.72
        return Qt.rect(origin.x, origin.y, size, size)
    }


    NumberAnimation on entryPhase {
        from: 0.0
        to: Math.PI * 2.0
        duration: 5200
        loops: Animation.Infinite
        running: root.visible
                 && !root.transitioning
                 && root.accessibilityMode === AuroraTypes.AccessibilityNormal
    }

    MangaBackdrop {
        anchors.fill: parent
        accentColor: root.homeIdentityColor
        secondaryColor: AuroraTokens.warmAccent
        energy: AudioRuntime.audioLevel * 0.46
                + (Moments.hasMoment ? 0.30 : 0.16)
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: root.homeIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.34 : 0.24
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        paperMode: true
        opacity: 0.24
    }

    Rectangle {
        anchors.centerIn: currentMoment
        width: currentMoment.width * 1.18
        height: width
        radius: width * 0.28
        rotation: -7
        color: Qt.rgba(root.homeIdentityColor.r,
                       root.homeIdentityColor.g,
                       root.homeIdentityColor.b,
                       0.035)
        border.width: 2
        border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                              AuroraTokens.mangaInk.g,
                              AuroraTokens.mangaInk.b,
                              0.08)
        opacity: root.transitioning ? 0.0 : 0.68

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }
    }

    Column {
        id: editorial
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Math.max(52, parent.width * 0.07)
        anchors.verticalCenterOffset: -18
        width: Math.min(430, parent.width * 0.34)
        spacing: 10

        Text {
            text: "AURORA"
            color: AuroraTokens.mangaInk
            font.pixelSize: 13
            font.letterSpacing: 4
            font.weight: Font.DemiBold
        }

        Rectangle {
            width: Math.min(132, parent.width * 0.38)
            height: 3
            radius: 2
            color: root.homeIdentityColor
            opacity: 0.72
        }

        Text {
            width: parent.width
            text: root.homeHeading
            color: AuroraTokens.mangaMuted
            font.pixelSize: 17
            font.weight: Font.Medium
            wrapMode: Text.Wrap
        }

        Text {
            width: parent.width
            text: root.homeTitle
            color: AuroraTokens.mangaInk
            font.pixelSize: Math.max(38, root.width * 0.040)
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.homeArtist.length > 0
            text: root.homeArtist
            color: AuroraTokens.mangaMuted
            font.pixelSize: 17
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.homeMeaning.length > 0
            text: "「" + root.homeMeaning + "」"
            color: AuroraTokens.mangaInk
            opacity: 0.76
            font.pixelSize: 18
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
            topPadding: 8
        }

        Text {
            width: parent.width
            visible: root.homeMeaning.length === 0
            text: root.homePeriod
            color: AuroraTokens.mangaMuted
            opacity: 0.72
            font.pixelSize: 14
        }
    }

    AuroraMoment {
        id: currentMoment

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: root.width >= 980 ? root.width * 0.17 : 0
        anchors.verticalCenterOffset: 12
        sizePreset: 1
        heroHomeMode: true
        title: root.homeHeading
        periodLabel: root.homePeriod
        trackTitle: root.homeTitle
        artist: root.homeArtist
        artworkSource: root.showingLatestMoment
                       ? Moments.latestArtworkSource
                       : AudioRuntime.hasTrack
                         ? AudioRuntime.artworkSource
                         : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        emotionColor: root.homeIdentityColor
        confirmedUserNote: root.homeMeaning
        availability: root.showingLatestMoment && !Moments.latestAvailable
                      ? AuroraTypes.AvailabilityDetached
                      : AuroraTypes.AvailabilityActive
        experienceState: root.transitioning
                         ? AuroraTypes.MomentRecalling
                         : root.showingLatestMoment
                           ? Moments.latestAvailable
                             ? root.homeMeaning.length > 0
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

    Item {
        anchors.horizontalCenter: currentMoment.horizontalCenter
        anchors.top: currentMoment.bottom
        anchors.topMargin: -24
        width: 96
        height: 44
        opacity: root.transitioning ? 0.0 : 0.72

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: 1
            height: 18 + Math.sin(root.entryPhase) * 3
            color: root.homeIdentityColor
            opacity: 0.62
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            text: "↗"
            color: AuroraTokens.mangaInk
            font.pixelSize: 17
            rotation: Math.sin(root.entryPhase * 0.7) * 2
        }

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        radius: 21
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: Moments.hasMoment
                      ? root.homeIdentityColor
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
}
