import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    property bool memoryPanelExpanded: false

    signal closeRequested()
    signal recallMoment(string momentId)
    signal manageSourcesRequested()

    function identityAnchorRect() {
        const origin = featuredMoment.mapToItem(
            root,
            featuredMoment.width * 0.30,
            featuredMoment.height * 0.12)
        const size = featuredMoment.width * 0.40
        return Qt.rect(origin.x, origin.y, size, size)
    }

    function syncMeaningEditor() {
        if (!meaningInput.activeFocus)
            meaningInput.text = Moments.selectedConfirmedMeaning
    }

    onVisibleChanged: {
        if (!visible)
            return
        if (Moments.selectedMomentId.length === 0
                && Moments.latestMomentId.length > 0) {
            Moments.selectMoment(Moments.latestMomentId)
        }
        syncMeaningEditor()
    }

    Connections {
        target: Moments

        function onSelectionChanged() {
            root.syncMeaningEditor()
        }
    }

    MangaBackdrop {
        anchors.fill: parent
        accentColor: Moments.selectedIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        energy: Moments.momentCount > 0 ? 0.26 : 0.12
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: Moments.selectedIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: 0.20
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        paperMode: true
        opacity: 0.16
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 28
        width: 88
        height: 42
        radius: 8
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: AuroraTokens.mangaInk

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("memory.back")
            color: AuroraTokens.mangaInk
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.closeRequested() }
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: Math.max(48, parent.width * 0.065)
        anchors.topMargin: 96
        spacing: 7

        Text {
            text: AuroraI18n.traditionalChinese ? "記憶" : "MEMORY"
            color: AuroraTokens.mangaInk
            font.pixelSize: 12
            font.letterSpacing: 4
            font.weight: Font.DemiBold
        }

        Text {
            text: AuroraI18n.text("memory.hero")
            color: AuroraTokens.mangaInk
            font.pixelSize: Math.max(30, root.width * 0.038)
            font.weight: Font.DemiBold
        }

        Text {
            text: Moments.momentCount === 1
                  ? AuroraI18n.text("memory.countOne")
                  : Moments.momentCount + " " + AuroraI18n.text("memory.countMany")
            color: AuroraTokens.mangaMuted
            font.pixelSize: 14
        }
    }

    AuroraMoment {
        id: featuredMoment

        x: root.memoryPanelExpanded
           ? Math.max(56, (memoryPanel.x - width) / 2)
           : (root.width - width) / 2
        anchors.verticalCenter: parent.verticalCenter
        sizePreset: root.width < 1080 ? 0 : 1
        title: Moments.selectedHeading
        periodLabel: Moments.selectedPeriodLabel
        trackTitle: Moments.selectedTitle
        artist: Moments.selectedArtist
        artworkSource: Moments.selectedArtworkSource
        emotionColor: Moments.selectedIdentityColor
        confirmedUserNote: Moments.selectedConfirmedMeaning
        availability: Moments.selectedAvailable
                      ? AuroraTypes.AvailabilityActive
                      : AuroraTypes.AvailabilityDetached
        experienceState: root.transitioning
                         ? AuroraTypes.MomentRecalling
                         : Moments.selectedAvailable
                           ? Moments.selectedConfirmedMeaning.length > 0
                             ? AuroraTypes.MomentMeaningful
                             : AuroraTypes.MomentRemembered
                           : AuroraTypes.MomentDetached
        identityVisible: root.identityVisible
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        mangaMode: true
        visible: Moments.momentCount > 0

        onActivated: {
            if (Moments.selectedAvailable)
                root.recallMoment(Moments.selectedMomentId)
            else
                root.manageSourcesRequested()
        }
        onRecallRequested: {
            if (Moments.selectedAvailable)
                root.recallMoment(Moments.selectedMomentId)
        }
        onRelinkRequested: root.manageSourcesRequested()
    }

    Text {
        anchors.horizontalCenter: featuredMoment.horizontalCenter
        anchors.top: featuredMoment.bottom
        anchors.topMargin: 12
        width: featuredMoment.width
        horizontalAlignment: Text.AlignHCenter
        text: Moments.selectedAvailable
              ? Moments.selectedCreatedLabel
              : AuroraI18n.text("memory.unavailable")
        color: Moments.selectedAvailable
               ? AuroraTokens.mangaMuted
               : AuroraTokens.warmAccent
        font.pixelSize: 11
        elide: Text.ElideRight
        visible: Moments.momentCount > 0
    }

    Rectangle {
        anchors.horizontalCenter: featuredMoment.horizontalCenter
        anchors.top: featuredMoment.bottom
        anchors.topMargin: 38
        width: 126
        height: 38
        radius: 8
        visible: Moments.momentCount > 0
        color: Moments.selectedAvailable
               ? Qt.rgba(Moments.selectedIdentityColor.r,
                         Moments.selectedIdentityColor.g,
                         Moments.selectedIdentityColor.b, 0.18)
               : AuroraTokens.mangaWash
        border.width: 2
        border.color: AuroraTokens.mangaInk

        Text {
            anchors.centerIn: parent
            text: Moments.selectedAvailable
                  ? AuroraI18n.text("memory.recall")
                  : AuroraI18n.text("memory.manageSources")
            color: AuroraTokens.mangaInk
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler {
            onTapped: {
                if (Moments.selectedAvailable)
                    root.recallMoment(Moments.selectedMomentId)
                else
                    root.manageSourcesRequested()
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: featuredMoment.horizontalCenter
        anchors.top: featuredMoment.bottom
        anchors.topMargin: 84
        width: 132
        height: 38
        radius: 8
        visible: Moments.momentCount > 0
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: AuroraTokens.mangaInk

        Text {
            anchors.centerIn: parent
            text: root.memoryPanelExpanded
                  ? AuroraI18n.text("memory.closeFlow")
                  : AuroraI18n.text("memory.openFlow")
            color: AuroraTokens.mangaInk
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.memoryPanelExpanded = !root.memoryPanelExpanded }
    }

    Rectangle {
        id: memoryPanel

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 28
        anchors.topMargin: 104
        width: Math.min(410, root.width * 0.34)
        radius: 12
        visible: root.memoryPanelExpanded
        enabled: root.memoryPanelExpanded
        opacity: root.memoryPanelExpanded ? 1.0 : 0.0
        color: AuroraTokens.mangaPanel
        border.width: 3
        border.color: AuroraTokens.mangaInk

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            Row {
                width: parent.width
                height: 28
                spacing: 8

                Text {
                    width: parent.width - 90
                    anchors.verticalCenter: parent.verticalCenter
                    text: AuroraI18n.text("memory.title")
                    color: AuroraTokens.mangaInk
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                Text {
                    width: 82
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignRight
                    text: Moments.momentCount
                    color: AuroraTokens.mangaMuted
                    font.pixelSize: 12
                }
            }

            ListView {
                id: momentList

                width: parent.width
                height: Math.max(170, parent.height * 0.48)
                model: Moments.momentItems
                spacing: 7
                clip: true

                delegate: Rectangle {
                    required property var modelData

                    width: momentList.width
                    height: 70
                    radius: 9
                    color: Moments.selectedMomentId === modelData.momentId
                           ? Qt.rgba(modelData.identityColor.r,
                                     modelData.identityColor.g,
                                     modelData.identityColor.b, 0.16)
                           : Qt.rgba(AuroraTokens.mangaInk.r,
                                     AuroraTokens.mangaInk.g,
                                     AuroraTokens.mangaInk.b,
                                     0.045)
                    border.width: 2
                    border.color: Moments.selectedMomentId === modelData.momentId
                                  ? modelData.identityColor
                                  : Qt.rgba(AuroraTokens.mangaInk.r,
                                            AuroraTokens.mangaInk.g,
                                            AuroraTokens.mangaInk.b,
                                            0.20)

                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10
                        width: 44
                        height: 44
                        radius: 8
                        color: Qt.rgba(modelData.identityColor.r,
                                       modelData.identityColor.g,
                                       modelData.identityColor.b, 0.22)
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: modelData.title.length > 0
                                  ? modelData.title.charAt(0).toUpperCase()
                                  : "M"
                            color: AuroraTokens.mangaInk
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.right: availabilityDot.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 66
                        anchors.rightMargin: 10
                        spacing: 4

                        Text {
                            width: parent.width
                            text: modelData.title
                            color: AuroraTokens.mangaInk
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: modelData.confirmedMeaning.length > 0
                                  ? modelData.confirmedMeaning
                                  : modelData.heading + " · " + modelData.artist
                            color: AuroraTokens.mangaMuted
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        id: availabilityDot
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 12
                        width: 7
                        height: 7
                        radius: 4
                        color: modelData.available
                               ? modelData.identityColor
                               : AuroraTokens.warmAccent
                    }

                    TapHandler {
                        onTapped: Moments.selectMoment(modelData.momentId)
                    }
                }

                Text {
                    anchors.centerIn: parent
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: AuroraI18n.text("memory.empty")
                    visible: momentList.count === 0
                    color: AuroraTokens.mangaMuted
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Qt.rgba(AuroraTokens.mangaInk.r,
                               AuroraTokens.mangaInk.g,
                               AuroraTokens.mangaInk.b,
                               0.18)
            }

            Text {
                text: AuroraI18n.text("memory.meaning")
                color: AuroraTokens.mangaInk
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }

            Rectangle {
                width: parent.width
                height: Math.max(92, parent.height - momentList.height - 144)
                radius: 9
                color: AuroraTokens.mangaPaper
                border.width: 2
                border.color: meaningInput.activeFocus
                              ? Moments.selectedIdentityColor
                              : AuroraTokens.mangaInk

                TextEdit {
                    id: meaningInput

                    anchors.fill: parent
                    anchors.margins: 11
                    color: AuroraTokens.mangaInk
                    selectionColor: Qt.rgba(
                                        Moments.selectedIdentityColor.r,
                                        Moments.selectedIdentityColor.g,
                                        Moments.selectedIdentityColor.b, 0.42)
                    selectedTextColor: AuroraTokens.mangaInk
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    enabled: Moments.selectedMomentId.length > 0
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 11
                    text: AuroraI18n.text("memory.meaningHint")
                    color: AuroraTokens.mangaMuted
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    visible: meaningInput.text.length === 0
                             && !meaningInput.activeFocus
                }
            }

            Row {
                width: parent.width
                height: 34
                spacing: 8

                Text {
                    width: parent.width - 122
                    anchors.verticalCenter: parent.verticalCenter
                    text: Moments.errorString.length > 0
                          ? Moments.errorString
                          : Moments.lastStatus
                    color: Moments.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : AuroraTokens.mangaMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Rectangle {
                    width: 114
                    height: 34
                    radius: 8
                    color: Moments.selectedMomentId.length > 0
                           ? Qt.rgba(Moments.selectedIdentityColor.r,
                                     Moments.selectedIdentityColor.g,
                                     Moments.selectedIdentityColor.b, 0.18)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: meaningInput.text.trim().length > 0
                              ? AuroraI18n.text("memory.saveMeaning")
                              : AuroraI18n.text("memory.clearMeaning")
                        color: AuroraTokens.mangaInk
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: Moments.selectedMomentId.length > 0
                        onTapped: {
                            Moments.updateConfirmedMeaning(
                                Moments.selectedMomentId,
                                meaningInput.text)
                            meaningInput.focus = false
                        }
                    }
                }
            }
        }
    }
}
