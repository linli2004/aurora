import QtQuick
import Aurora.Runtime 1.0

FocusScope {
    id: root

    // 0 Compact, 1 Primary, 2 Recall
    property int sizePreset: 1
    property int experienceState: AuroraTypes.MomentPresent
    property int context: AuroraTypes.Home
    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property int availability: AuroraTypes.AvailabilityActive
    property bool recallEnabled: true
    property bool identityVisible: true
    property bool mangaMode: false

    property string momentId: "moment-demo"
    property string title: "Late Summer Night"
    property string artist: "Aurora Demo"
    property string trackTitle: "Quiet Signals"
    property string periodLabel: "Late Summer Night"
    property string confirmedUserNote: ""
    property bool hasConfirmedMeaning: confirmedUserNote.length > 0
    property url artworkSource: ""
    property color emotionColor: AuroraTokens.coolAccent

    signal activated()
    signal recallRequested()
    signal keepMomentRequested()
    signal relinkRequested()

    readonly property real componentSize: AuroraTokens.momentSize(sizePreset)
    readonly property bool detached:
        availability === AuroraTypes.AvailabilityDetached
        || experienceState === AuroraTypes.MomentDetached
    readonly property bool archived: experienceState === AuroraTypes.MomentArchived
    readonly property bool dormant: experienceState === AuroraTypes.MomentDormant
    readonly property bool recalling: experienceState === AuroraTypes.MomentRecalling
    readonly property bool remembered:
        experienceState === AuroraTypes.MomentRemembered
        || experienceState === AuroraTypes.MomentMeaningful
        || experienceState === AuroraTypes.MomentRecalling
    readonly property bool ambientEnabled:
        accessibilityMode === AuroraTypes.AccessibilityNormal
        && qualityMode !== AuroraTypes.Eco
        && !archived
        && !dormant
    readonly property color semanticAccent: AuroraTokens.momentAccent(experienceState)
    readonly property real semanticPresence: AuroraTokens.momentPresence(experienceState)
    readonly property real semanticOpacity: AuroraTokens.momentOpacity(experienceState)
    readonly property color resolvedPrimaryText:
        root.mangaMode ? AuroraTokens.mangaInk : AuroraTokens.textPrimary
    readonly property color resolvedSecondaryText:
        root.mangaMode ? AuroraTokens.mangaMuted : AuroraTokens.textSecondary
    readonly property color resolvedMutedText:
        root.mangaMode ? Qt.rgba(AuroraTokens.mangaInk.r,
                                 AuroraTokens.mangaInk.g,
                                 AuroraTokens.mangaInk.b,
                                 0.54)
                       : AuroraTokens.textMuted
    readonly property string trackArtistLine:
        root.artist.length > 0
        ? root.trackTitle + "  ·  " + root.artist
        : root.trackTitle

    width: componentSize
    height: componentSize
    opacity: semanticOpacity
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: title + ", " + trackTitle + " by " + artist
    Accessible.description: detached
                            ? "Music source unavailable. Activate to relink."
                            : archived
                              ? "Archived music moment. Activate to recall."
                              : "Activate to enter this music moment."

    Behavior on opacity {
        NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
    }

    Keys.onReturnPressed: detached ? relinkRequested() : activated()
    Keys.onEnterPressed: detached ? relinkRequested() : activated()
    Keys.onSpacePressed: detached ? relinkRequested() : activated()

    Rectangle {
        id: surface
        anchors.fill: parent
        radius: AuroraTokens.momentRadius(root.sizePreset)
        color: root.mangaMode
               ? AuroraTokens.mangaPanel
               : root.accessibilityMode === AuroraTypes.HighContrast
               ? AuroraTokens.surfaceHighContrast
               : AuroraTokens.surfacePrimary
        border.width: root.mangaMode ? (root.activeFocus ? 4 : 3)
                                     : root.activeFocus ? 3 : (root.remembered ? 2 : 1)
        border.color: root.activeFocus
                      ? AuroraTokens.focusRing
                      : root.mangaMode
                        ? AuroraTokens.mangaInk
                      : root.remembered
                        ? Qt.rgba(root.semanticAccent.r,
                                  root.semanticAccent.g,
                                  root.semanticAccent.b,
                                  0.34)
                        : AuroraTokens.lineSubtle
        clip: true
        transform: Translate { y: hoverHandler.hovered ? -4 : 0 }
        scale: hoverHandler.hovered && root.accessibilityMode !== AuroraTypes.ReducedMotion ? 1.015 : 1.0

        Behavior on scale {
            NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
        }

        AtmosphereField {
            anchors.fill: parent
            primaryColor: root.emotionColor
            secondaryColor: root.semanticAccent
            presenceLevel: root.semanticPresence
            accessibilityMode: root.accessibilityMode
            qualityMode: root.qualityMode
            paperMode: root.mangaMode
            opacity: root.mangaMode ? 0.16 : 1.0
        }

        Repeater {
            model: root.mangaMode ? 9 : 0

            delegate: Rectangle {
                required property int index

                width: parent.width * (0.24 + index * 0.035)
                height: 2
                radius: 1
                x: parent.width * (0.03 + index * 0.038)
                y: parent.height * (0.16 + index * 0.055)
                rotation: -19
                color: AuroraTokens.mangaInk
                opacity: 0.05 + root.semanticPresence * 0.05
            }
        }

        // Temporal trace: a restrained horizontal memory mark, never a rarity badge.
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: parent.width * 0.09
            anchors.topMargin: parent.height * 0.075
            width: parent.width * (root.remembered ? 0.28 : 0.14)
            height: Math.max(1, parent.width * 0.006)
            radius: height / 2
            color: root.mangaMode ? AuroraTokens.mangaInk : root.semanticAccent
            opacity: root.archived ? 0.20 : root.remembered ? 0.62 : 0.28

            Behavior on width {
                enabled: root.accessibilityMode !== AuroraTypes.ReducedMotion
                NumberAnimation { duration: AuroraTokens.motionFlow; easing.type: Easing.OutCubic }
            }
        }

        // Atmosphere Safe Zone / music identity reference.
        AuroraCrystal {
            id: crystal
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.12
            crystalSize: parent.width * 0.40
            title: root.trackTitle
            artist: root.artist
            artworkSource: root.artworkSource
            colorSignature: root.emotionColor
            experienceState: root.detached
                             ? AuroraTypes.CrystalDetached
                             : root.recalling
                               ? AuroraTypes.CrystalTransitioning
                               : AuroraTypes.CrystalPresent
            context: AuroraTypes.Moment
            accessibilityMode: root.accessibilityMode
            qualityMode: root.qualityMode
            heroMode: root.mangaMode
            mangaMode: root.mangaMode
            opacity: root.identityVisible ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation {
                    duration: AuroraTokens.motionSoft
                    easing.type: Easing.OutCubic
                }
            }

            onRelinkRequested: root.relinkRequested()
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: parent.width * 0.09
            spacing: Math.max(6, parent.width * 0.018)

            Text {
                width: parent.width
                text: root.periodLabel
                color: root.resolvedSecondaryText
                opacity: root.archived ? 0.62 : 1.0
                font.pixelSize: Math.max(12, root.width * 0.035)
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: root.title
                color: root.resolvedPrimaryText
                opacity: root.archived ? 0.72 : 1.0
                font.pixelSize: Math.max(22, root.width * 0.065)
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: root.trackArtistLine
                color: root.resolvedSecondaryText
                opacity: root.archived ? 0.58 : 1.0
                font.pixelSize: Math.max(13, root.width * 0.036)
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                visible: root.hasConfirmedMeaning
                text: root.confirmedUserNote
                color: root.resolvedPrimaryText
                opacity: root.archived ? 0.56 : 0.78
                font.pixelSize: Math.max(12, root.width * 0.033)
                maximumLineCount: 1
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                visible: root.detached
                text: "Music source detached · Activate to relink"
                color: AuroraTokens.focusRing
                font.pixelSize: Math.max(12, root.width * 0.032)
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                visible: root.archived
                text: "Archived · Recall remains available"
                color: root.resolvedMutedText
                font.pixelSize: Math.max(12, root.width * 0.032)
                elide: Text.ElideRight
            }
        }

        Rectangle {
            visible: root.experienceState === AuroraTypes.MomentMeaningful
            width: parent.width * 0.028
            height: width
            radius: width / 2
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: parent.width * 0.08
            anchors.topMargin: parent.height * 0.07
            color: AuroraTokens.memoryAccent
            opacity: 0.88
        }

        SequentialAnimation on opacity {
            running: root.recalling
                     && root.ambientEnabled
                     && root.accessibilityMode !== AuroraTypes.ReducedMotion
            loops: Animation.Infinite
            NumberAnimation { to: 0.88; duration: 1100; easing.type: Easing.InOutSine }
            NumberAnimation { to: root.semanticOpacity; duration: 1100; easing.type: Easing.InOutSine }
        }
    }

    HoverHandler { id: hoverHandler }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: root.detached ? root.relinkRequested() : root.activated()
        onLongPressed: {
            if (root.recallEnabled && !root.detached)
                root.recallRequested()
        }
    }
}
