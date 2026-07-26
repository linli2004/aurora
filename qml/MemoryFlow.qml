import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    property bool memoryPanelExpanded: false

    readonly property bool hasSelection: Moments.selectedMomentId.length > 0
    readonly property bool selectedDetached:
        root.hasSelection && !Moments.selectedAvailable

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
        energy: Moments.momentCount > 0 ? 0.23 : 0.10
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: Moments.selectedIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: root.hasSelection ? 0.22 : 0.14
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        paperMode: true
        opacity: 0.17
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
        z: 20

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("memory.back")
            color: AuroraTokens.mangaInk
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.closeRequested() }
    }

    Row {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        height: 42
        spacing: 10
        z: 20

        Rectangle {
            width: 136
            height: 42
            radius: 21
            color: Qt.rgba(Moments.selectedIdentityColor.r,
                           Moments.selectedIdentityColor.g,
                           Moments.selectedIdentityColor.b,
                           0.10)
            border.width: 1
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.28)

            Text {
                anchors.centerIn: parent
                text: AuroraI18n.text("memory.localPrivate")
                color: AuroraTokens.mangaMuted
                font.pixelSize: 11
                font.weight: Font.Medium
            }
        }

        Rectangle {
            width: 110
            height: 42
            radius: 21
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: Moments.selectedIdentityColor

            Text {
                anchors.centerIn: parent
                text: AuroraI18n.text("home.memory")
                      + " · " + Moments.momentCount
                color: AuroraTokens.mangaInk
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
        }
    }

    Column {
        id: editorial

        anchors.left: parent.left
        anchors.leftMargin: Math.max(48, parent.width * 0.065)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -42
        width: Math.min(400, parent.width * 0.34)
        spacing: 9

        Text {
            text: AuroraI18n.traditionalChinese ? "記憶流" : "MEMORY FLOW"
            color: AuroraTokens.mangaInk
            font.pixelSize: 12
            font.letterSpacing: 4
            font.weight: Font.DemiBold
        }

        Rectangle {
            width: Math.min(132, parent.width * 0.42)
            height: 3
            radius: 2
            color: Moments.selectedIdentityColor
            opacity: 0.72
        }

        Text {
            width: parent.width
            text: root.hasSelection
                  ? AuroraI18n.momentHeading(Moments.selectedHeading)
                  : AuroraI18n.text("memory.hero")
            color: AuroraTokens.mangaMuted
            font.pixelSize: 16
            font.weight: Font.Medium
            wrapMode: Text.Wrap
        }

        Text {
            width: parent.width
            text: root.hasSelection
                  ? Moments.selectedTitle
                  : AuroraI18n.text("memory.empty")
            color: AuroraTokens.mangaInk
            font.pixelSize: Math.max(34, root.width * 0.037)
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.hasSelection && Moments.selectedArtist.length > 0
            text: Moments.selectedArtist
            color: AuroraTokens.mangaMuted
            font.pixelSize: 16
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            visible: root.hasSelection
                     && Moments.selectedConfirmedMeaning.length > 0
            text: "「" + Moments.selectedConfirmedMeaning + "」"
            color: AuroraTokens.mangaInk
            opacity: 0.80
            font.pixelSize: 18
            wrapMode: Text.Wrap
            maximumLineCount: 4
            elide: Text.ElideRight
            topPadding: 8
        }

        Text {
            width: parent.width
            visible: root.hasSelection
                     && Moments.selectedConfirmedMeaning.length === 0
            text: AuroraI18n.text("memory.selectionHint")
            color: AuroraTokens.mangaMuted
            opacity: 0.72
            font.pixelSize: 13
            wrapMode: Text.Wrap
            topPadding: 7
        }

        Rectangle {
            width: parent.width
            height: 54
            radius: 10
            color: root.selectedDetached
                   ? Qt.rgba(AuroraTokens.warmAccent.r,
                             AuroraTokens.warmAccent.g,
                             AuroraTokens.warmAccent.b,
                             0.09)
                   : Qt.rgba(Moments.selectedIdentityColor.r,
                             Moments.selectedIdentityColor.g,
                             Moments.selectedIdentityColor.b,
                             0.08)
            border.width: 1
            border.color: root.selectedDetached
                          ? AuroraTokens.warmAccent
                          : Qt.rgba(Moments.selectedIdentityColor.r,
                                    Moments.selectedIdentityColor.g,
                                    Moments.selectedIdentityColor.b,
                                    0.42)
            visible: root.hasSelection

            Row {
                anchors.fill: parent
                anchors.margins: 11
                spacing: 10

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 8
                    height: 8
                    radius: 4
                    color: root.selectedDetached
                           ? AuroraTokens.warmAccent
                           : Moments.selectedIdentityColor
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 18
                    text: root.selectedDetached
                          ? AuroraI18n.text("memory.sourceDetached")
                          : AuroraI18n.text("memory.sourceReady")
                    color: root.selectedDetached
                           ? AuroraTokens.warmAccent
                           : AuroraTokens.mangaMuted
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }
        }

        Row {
            width: parent.width
            height: 42
            spacing: 10
            visible: root.hasSelection

            Rectangle {
                width: Math.min(150, (parent.width - 10) * 0.5)
                height: 42
                radius: 9
                color: root.selectedDetached
                       ? AuroraTokens.mangaWash
                       : Qt.rgba(Moments.selectedIdentityColor.r,
                                 Moments.selectedIdentityColor.g,
                                 Moments.selectedIdentityColor.b,
                                 0.20)
                border.width: 2
                border.color: AuroraTokens.mangaInk

                Text {
                    anchors.centerIn: parent
                    text: root.selectedDetached
                          ? AuroraI18n.text("memory.manageSources")
                          : AuroraI18n.text("memory.recall")
                    color: AuroraTokens.mangaInk
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }

                TapHandler {
                    onTapped: {
                        if (root.selectedDetached)
                            root.manageSourcesRequested()
                        else
                            root.recallMoment(Moments.selectedMomentId)
                    }
                }
            }

            Rectangle {
                width: Math.min(150, (parent.width - 10) * 0.5)
                height: 42
                radius: 9
                color: AuroraTokens.mangaPanel
                border.width: 2
                border.color: AuroraTokens.mangaInk

                Text {
                    anchors.centerIn: parent
                    text: AuroraI18n.text("memory.editMeaning")
                    color: AuroraTokens.mangaInk
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }

                TapHandler {
                    onTapped: {
                        root.memoryPanelExpanded = true
                        root.syncMeaningEditor()
                    }
                }
            }
        }
    }

    AuroraMoment {
        id: featuredMoment

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: root.width >= 980
                                        ? root.width * 0.18
                                        : root.width * 0.10
        anchors.verticalCenterOffset: -28
        sizePreset: root.width < 1080 ? 0 : 1
        title: AuroraI18n.momentHeading(Moments.selectedHeading)
        periodLabel: AuroraI18n.momentPeriod(Moments.selectedPeriodLabel)
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
        opacity: root.memoryPanelExpanded ? 0.26 : 1.0
        scale: root.memoryPanelExpanded ? 0.94 : 1.0

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

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }
    }

    Rectangle {
        id: timeline

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        anchors.bottomMargin: 24
        height: 128
        radius: 14
        color: Qt.rgba(AuroraTokens.mangaPanel.r,
                       AuroraTokens.mangaPanel.g,
                       AuroraTokens.mangaPanel.b,
                       0.86)
        border.width: 1
        border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                              AuroraTokens.mangaInk.g,
                              AuroraTokens.mangaInk.b,
                              0.20)
        z: 12

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.topMargin: 12
            height: 22

            Text {
                width: parent.width - 110
                text: AuroraI18n.text("memory.timeline")
                color: AuroraTokens.mangaInk
                font.pixelSize: 12
                font.weight: Font.DemiBold
                font.letterSpacing: 1.4
            }

            Text {
                width: 110
                horizontalAlignment: Text.AlignRight
                text: Moments.momentCount === 1
                      ? AuroraI18n.text("memory.countOne")
                      : Moments.momentCount + " "
                        + AuroraI18n.text("memory.countMany")
                color: AuroraTokens.mangaMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }

        ListView {
            id: momentTimeline

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.topMargin: 38
            anchors.bottomMargin: 10
            orientation: ListView.Horizontal
            model: Moments.momentItems
            spacing: 8
            clip: true

            delegate: Item {
                required property var modelData

                width: 174
                height: momentTimeline.height

                readonly property bool selected:
                    Moments.selectedMomentId === modelData.momentId

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: node.verticalCenter
                    height: 1
                    color: Qt.rgba(AuroraTokens.mangaInk.r,
                                   AuroraTokens.mangaInk.g,
                                   AuroraTokens.mangaInk.b,
                                   0.16)
                }

                Rectangle {
                    id: node

                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.selected ? 18 : 12
                    height: width
                    radius: width / 2
                    color: modelData.available
                           ? modelData.identityColor
                           : AuroraTokens.warmAccent
                    border.width: parent.selected ? 3 : 2
                    border.color: AuroraTokens.mangaPaper
                    scale: parent.selected ? 1.08 : 1.0

                    Behavior on width {
                        NumberAnimation {
                            duration: AuroraTokens.motionSoft
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                Column {
                    anchors.left: node.right
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3

                    Text {
                        width: parent.width
                        text: AuroraI18n.momentPeriod(modelData.periodLabel)
                        color: modelData.identityColor
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: modelData.title
                        color: AuroraTokens.mangaInk
                        font.pixelSize: parent.parent.selected ? 13 : 12
                        font.weight: parent.parent.selected
                                     ? Font.DemiBold
                                     : Font.Medium
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: modelData.confirmedMeaning.length > 0
                              ? modelData.confirmedMeaning
                              : modelData.artist
                        color: AuroraTokens.mangaMuted
                        font.pixelSize: 9
                        elide: Text.ElideRight
                    }
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
                visible: momentTimeline.count === 0
                color: AuroraTokens.mangaMuted
                font.pixelSize: 12
                wrapMode: Text.Wrap
            }
        }
    }

    Rectangle {
        id: meaningPanel

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: timeline.top
        anchors.margins: 28
        anchors.topMargin: 92
        anchors.bottomMargin: 16
        width: Math.min(420, root.width * 0.38)
        radius: 14
        visible: root.memoryPanelExpanded
        enabled: root.memoryPanelExpanded
        opacity: root.memoryPanelExpanded ? 1.0 : 0.0
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: AuroraTokens.mangaInk
        z: 30

        Behavior on opacity {
            NumberAnimation {
                duration: AuroraTokens.motionSoft
                easing.type: Easing.OutCubic
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Row {
                width: parent.width
                height: 36
                spacing: 8

                Column {
                    width: parent.width - 100
                    spacing: 2

                    Text {
                        text: AuroraI18n.text("memory.meaning")
                        color: AuroraTokens.mangaInk
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    Text {
                        width: parent.width
                        text: Moments.selectedTitle
                        color: AuroraTokens.mangaMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    width: 92
                    height: 34
                    radius: 8
                    color: AuroraTokens.mangaWash
                    border.width: 1
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("memory.closeEditor")
                        color: AuroraTokens.mangaInk
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        onTapped: {
                            meaningInput.focus = false
                            root.memoryPanelExpanded = false
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height - 120
                radius: 10
                color: AuroraTokens.mangaPaper
                border.width: 2
                border.color: meaningInput.activeFocus
                              ? Moments.selectedIdentityColor
                              : AuroraTokens.mangaInk

                TextEdit {
                    id: meaningInput

                    anchors.fill: parent
                    anchors.margins: 13
                    color: AuroraTokens.mangaInk
                    selectionColor: Qt.rgba(
                                        Moments.selectedIdentityColor.r,
                                        Moments.selectedIdentityColor.g,
                                        Moments.selectedIdentityColor.b,
                                        0.42)
                    selectedTextColor: AuroraTokens.mangaInk
                    font.pixelSize: 14
                    wrapMode: TextEdit.Wrap
                    enabled: Moments.selectedMomentId.length > 0
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 13
                    text: AuroraI18n.text("memory.meaningHint")
                    color: AuroraTokens.mangaMuted
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    visible: meaningInput.text.length === 0
                             && !meaningInput.activeFocus
                }
            }

            Row {
                width: parent.width
                height: 38
                spacing: 8

                Text {
                    width: parent.width - 128
                    anchors.verticalCenter: parent.verticalCenter
                    text: Moments.errorString.length > 0
                          ? AuroraI18n.text("memory.sourceDetached")
                          : Moments.lastStatus
                    color: Moments.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : AuroraTokens.mangaMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Rectangle {
                    width: 120
                    height: 38
                    radius: 8
                    color: Moments.selectedMomentId.length > 0
                           ? Qt.rgba(Moments.selectedIdentityColor.r,
                                     Moments.selectedIdentityColor.g,
                                     Moments.selectedIdentityColor.b,
                                     0.18)
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
