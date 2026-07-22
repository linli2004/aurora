import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property int coreStateIndex: 0
    property int momentStateIndex: 1
    property int crystalStateIndex: 1

    signal closeRequested()

    readonly property var coreStates: [
        AuroraTypes.CoreIdle,
        AuroraTypes.CorePlaying,
        AuroraTypes.CorePaused,
        AuroraTypes.CoreGathering,
        AuroraTypes.CoreTransition,
        AuroraTypes.CoreRecall
    ]
    readonly property var coreStateNames: [
        "Idle", "Playing", "Paused", "Gathering", "Transition", "Recall"
    ]
    readonly property var coreStateDescriptions: [
        "Quiet presence without demanding attention.",
        "Music is present; motion follows semantic energy.",
        "Motion is frozen while the relationship remains.",
        "Runtime state is gathering into a readable form.",
        "The perceptual state is changing between contexts.",
        "A previous atmosphere is being restored."
    ]

    readonly property var momentStates: [
        AuroraTypes.MomentDormant,
        AuroraTypes.MomentPresent,
        AuroraTypes.MomentRemembered,
        AuroraTypes.MomentMeaningful,
        AuroraTypes.MomentRecalling,
        AuroraTypes.MomentArchived,
        AuroraTypes.MomentDetached
    ]
    readonly property var momentStateNames: [
        "Dormant", "Present", "Remembered", "Meaningful", "Recalling", "Archived", "Detached"
    ]
    readonly property var momentStateDescriptions: [
        "An experience exists but has not entered memory.",
        "The experience is perceptually available now.",
        "The experience belongs to the memory system.",
        "The user has explicitly attached personal meaning.",
        "The saved context is being restored for re-entry.",
        "The memory remains owned but leaves daily flow.",
        "The music source is unavailable; memory remains."
    ]

    readonly property var crystalStates: [
        AuroraTypes.CrystalDormant,
        AuroraTypes.CrystalPresent,
        AuroraTypes.CrystalFocused,
        AuroraTypes.CrystalImmersive,
        AuroraTypes.CrystalTransitioning,
        AuroraTypes.CrystalDetached,
        AuroraTypes.CrystalUnavailable
    ]
    readonly property var crystalStateNames: [
        "Dormant", "Present", "Focused", "Immersive", "Transitioning", "Detached", "Unavailable"
    ]
    readonly property var crystalStateDescriptions: [
        "Music identity exists without active emphasis.",
        "The default recognizable music identity.",
        "The identity is selected or approached.",
        "The identity becomes the Music Space anchor.",
        "Recognition is preserved across contexts.",
        "The source is missing while identity persists.",
        "A trusted generated identity replaces missing art."
    ]

    readonly property string accessibilityName:
        accessibilityMode === AuroraTypes.ReducedMotion ? "Reduced Motion"
        : accessibilityMode === AuroraTypes.HighContrast ? "High Contrast"
        : accessibilityMode === AuroraTypes.CognitiveMinimal ? "Cognitive Minimal"
        : "Normal"
    readonly property string qualityName:
        qualityMode === AuroraTypes.Eco ? "Eco"
        : qualityMode === AuroraTypes.Immersive ? "Immersive"
        : qualityMode === AuroraTypes.Adaptive ? "Adaptive"
        : "Balanced"
    readonly property real panelWidth: Math.max(272, Math.min(318, (width - 112) / 3))
    readonly property real panelHeight: Math.min(520, Math.max(470, height - 246))

    function previousIndex(index, count) {
        return (index - 1 + count) % count
    }

    function nextIndex(index, count) {
        return (index + 1) % count
    }

    function cycleAccessibility() {
        if (accessibilityMode === AuroraTypes.AccessibilityNormal)
            accessibilityMode = AuroraTypes.ReducedMotion
        else if (accessibilityMode === AuroraTypes.ReducedMotion)
            accessibilityMode = AuroraTypes.HighContrast
        else if (accessibilityMode === AuroraTypes.HighContrast)
            accessibilityMode = AuroraTypes.CognitiveMinimal
        else
            accessibilityMode = AuroraTypes.AccessibilityNormal
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: AuroraTokens.memoryAccent
        secondaryColor: AuroraTokens.coolAccent
        presenceLevel: 0.12
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.02, 0.025, 0.04, 0.28)
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 32
        anchors.topMargin: 28
        spacing: 5

        Text {
            text: "Aurora Component Gallery"
            color: AuroraTokens.textPrimary
            font.pixelSize: 28
            font.weight: Font.DemiBold
        }

        Text {
            text: "Runtime states, identity continuity and accessibility baselines"
            color: AuroraTokens.textSecondary
            font.pixelSize: 14
        }
    }

    GalleryButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 28
        anchors.topMargin: 28
        buttonWidth: 94
        text: "Close"
        onActivated: root.closeRequested()
    }

    Row {
        id: previews
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 108
        spacing: 20

        Rectangle {
            width: root.panelWidth
            height: root.panelHeight
            radius: 28
            color: root.accessibilityMode === AuroraTypes.HighContrast
                   ? AuroraTokens.surfaceHighContrast
                   : AuroraTokens.surfacePanel
            border.width: 1
            border.color: AuroraTokens.lineSubtle

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 22
                spacing: 5

                Text {
                    text: "AURORA CORE"
                    color: AuroraTokens.textMuted
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.DemiBold
                }

                Text {
                    text: root.coreStateNames[root.coreStateIndex]
                    color: AuroraTokens.textPrimary
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Text {
                    width: parent.width
                    text: root.coreStateDescriptions[root.coreStateIndex]
                    color: AuroraTokens.textSecondary
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }

            AuroraCore {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 6
                diameter: 112
                experienceState: root.coreStates[root.coreStateIndex]
                memoryState: experienceState === AuroraTypes.CoreRecall
                accessibilityMode: root.accessibilityMode
                qualityMode: root.qualityMode
                onRequestPlayPause: root.coreStateIndex = root.nextIndex(root.coreStateIndex, root.coreStates.length)
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                spacing: 10

                GalleryButton {
                    buttonWidth: 82
                    text: "Previous"
                    onActivated: root.coreStateIndex = root.previousIndex(root.coreStateIndex, root.coreStates.length)
                }
                StateCounter {
                    current: root.coreStateIndex + 1
                    total: root.coreStates.length
                }
                GalleryButton {
                    buttonWidth: 82
                    text: "Next"
                    onActivated: root.coreStateIndex = root.nextIndex(root.coreStateIndex, root.coreStates.length)
                }
            }
        }

        Rectangle {
            width: root.panelWidth
            height: root.panelHeight
            radius: 28
            color: root.accessibilityMode === AuroraTypes.HighContrast
                   ? AuroraTokens.surfaceHighContrast
                   : AuroraTokens.surfacePanel
            border.width: 1
            border.color: AuroraTokens.lineSubtle

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 22
                spacing: 5

                Text {
                    text: "AURORA MOMENT"
                    color: AuroraTokens.textMuted
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.DemiBold
                }

                Text {
                    text: root.momentStateNames[root.momentStateIndex]
                    color: AuroraTokens.textPrimary
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Text {
                    width: parent.width
                    text: root.momentStateDescriptions[root.momentStateIndex]
                    color: AuroraTokens.textSecondary
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }

            AuroraMoment {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 12
                sizePreset: 0
                scale: Math.min(1.0, (parent.width - 38) / width)
                experienceState: root.momentStates[root.momentStateIndex]
                availability: experienceState === AuroraTypes.MomentDetached
                              ? AuroraTypes.AvailabilityDetached
                              : experienceState === AuroraTypes.MomentArchived
                                ? AuroraTypes.AvailabilityArchived
                                : AuroraTypes.AvailabilityActive
                title: "A remembered evening"
                periodLabel: "Late Summer Night"
                trackTitle: "Quiet Signals"
                artist: "Aurora Demo"
                artworkSource: "qrc:/qt/qml/Aurora/App/assets/demo-cover-b.png"
                confirmedUserNote: experienceState === AuroraTypes.MomentMeaningful ? "First apartment" : ""
                accessibilityMode: root.accessibilityMode
                qualityMode: root.qualityMode
                onActivated: root.momentStateIndex = root.nextIndex(root.momentStateIndex, root.momentStates.length)
                onRelinkRequested: root.momentStateIndex = 1
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                spacing: 10

                GalleryButton {
                    buttonWidth: 82
                    text: "Previous"
                    onActivated: root.momentStateIndex = root.previousIndex(root.momentStateIndex, root.momentStates.length)
                }
                StateCounter {
                    current: root.momentStateIndex + 1
                    total: root.momentStates.length
                }
                GalleryButton {
                    buttonWidth: 82
                    text: "Next"
                    onActivated: root.momentStateIndex = root.nextIndex(root.momentStateIndex, root.momentStates.length)
                }
            }
        }

        Rectangle {
            width: root.panelWidth
            height: root.panelHeight
            radius: 28
            color: root.accessibilityMode === AuroraTypes.HighContrast
                   ? AuroraTokens.surfaceHighContrast
                   : AuroraTokens.surfacePanel
            border.width: 1
            border.color: AuroraTokens.lineSubtle

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 22
                spacing: 5

                Text {
                    text: "AURORA CRYSTAL"
                    color: AuroraTokens.textMuted
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.weight: Font.DemiBold
                }

                Text {
                    text: root.crystalStateNames[root.crystalStateIndex]
                    color: AuroraTokens.textPrimary
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Text {
                    width: parent.width
                    text: root.crystalStateDescriptions[root.crystalStateIndex]
                    color: AuroraTokens.textSecondary
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }
            }

            AuroraCrystal {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 8
                crystalSize: Math.min(232, parent.width - 42)
                experienceState: root.crystalStates[root.crystalStateIndex]
                context: experienceState === AuroraTypes.CrystalImmersive
                         ? AuroraTypes.MusicSpace
                         : AuroraTypes.Home
                title: "Quiet Signals"
                artist: "Aurora Demo"
                artworkSource: experienceState === AuroraTypes.CrystalUnavailable
                               ? ""
                               : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
                colorSignature: AuroraTokens.coolAccent
                accessibilityMode: root.accessibilityMode
                qualityMode: root.qualityMode
                onRelinkRequested: root.crystalStateIndex = 1
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                spacing: 10

                GalleryButton {
                    buttonWidth: 82
                    text: "Previous"
                    onActivated: root.crystalStateIndex = root.previousIndex(root.crystalStateIndex, root.crystalStates.length)
                }
                StateCounter {
                    current: root.crystalStateIndex + 1
                    total: root.crystalStates.length
                }
                GalleryButton {
                    buttonWidth: 82
                    text: "Next"
                    onActivated: root.crystalStateIndex = root.nextIndex(root.crystalStateIndex, root.crystalStates.length)
                }
            }
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        spacing: 12

        GalleryButton {
            buttonWidth: 190
            text: "Accessibility: " + root.accessibilityName
            onActivated: root.cycleAccessibility()
        }
        GalleryButton {
            buttonWidth: 150
            text: "Quality: " + root.qualityName
            onActivated: root.qualityMode = root.qualityMode === AuroraTypes.Eco
                         ? AuroraTypes.Balanced
                         : AuroraTypes.Eco
        }
        GalleryButton {
            buttonWidth: 112
            text: "Reset"
            onActivated: {
                root.coreStateIndex = 0
                root.momentStateIndex = 1
                root.crystalStateIndex = 1
                root.accessibilityMode = AuroraTypes.AccessibilityNormal
                root.qualityMode = AuroraTypes.Balanced
            }
        }
    }

    component StateCounter: Rectangle {
        property int current: 1
        property int total: 1
        width: 48
        height: 36
        radius: 18
        color: Qt.rgba(1, 1, 1, 0.045)
        border.width: 1
        border.color: AuroraTokens.lineQuiet

        Text {
            anchors.centerIn: parent
            text: parent.current + "/" + parent.total
            color: AuroraTokens.textMuted
            font.pixelSize: 12
        }
    }

    component GalleryButton: FocusScope {
        id: button
        property string text: "Button"
        property real buttonWidth: 150
        signal activated()
        width: buttonWidth
        height: 36
        activeFocusOnTab: true
        Accessible.role: Accessible.Button
        Accessible.name: text
        Keys.onReturnPressed: activated()
        Keys.onEnterPressed: activated()
        Keys.onSpacePressed: activated()

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: Qt.rgba(1, 1, 1, button.activeFocus ? 0.13 : 0.065)
            border.width: button.activeFocus ? 2 : 1
            border.color: button.activeFocus ? AuroraTokens.focusRing : AuroraTokens.lineSubtle
        }
        Text {
            anchors.centerIn: parent
            text: button.text
            color: AuroraTokens.textSecondary
            font.pixelSize: 12
            elide: Text.ElideRight
            width: parent.width - 16
            horizontalAlignment: Text.AlignHCenter
        }
        TapHandler { onTapped: button.activated() }
    }
}
