pragma Singleton

import QtQuick
import Aurora.Runtime 1.0

QtObject {
    // Color tokens
    readonly property color windowBackground: "#090A0D"
    readonly property color surfacePrimary: "#151821"
    readonly property color surfaceRaised: "#1C202B"
    readonly property color surfacePanel: "#12151D"
    readonly property color surfaceHighContrast: "#000000"
    readonly property color surfaceOverlay: "#0D1017"
    readonly property color textPrimary: "#F4F3EF"
    readonly property color textSecondary: "#B6B8C2"
    readonly property color textMuted: "#858995"
    readonly property color focusRing: "#F1C6A8"
    readonly property color warmAccent: "#C9876A"
    readonly property color coolAccent: "#6E82B7"
    readonly property color memoryAccent: "#B39BCB"
    readonly property color detachedAccent: "#9A927E"
    readonly property color lineSubtle: Qt.rgba(1, 1, 1, 0.09)
    readonly property color lineQuiet: Qt.rgba(1, 1, 1, 0.045)
    readonly property color mangaPaper: "#F5EBD8"
    readonly property color mangaPanel: "#FFF8EA"
    readonly property color mangaWash: "#E6D5B9"
    readonly property color mangaInk: "#211D1A"
    readonly property color mangaMuted: "#70685F"

    // Spatial tokens
    readonly property int spacing4: 4
    readonly property int spacing8: 8
    readonly property int spacing12: 12
    readonly property int spacing16: 16
    readonly property int spacing24: 24
    readonly property int spacing32: 32
    readonly property int spacing48: 48
    readonly property int spacing64: 64

    // Motion tokens
    readonly property int motionSoft: 300
    readonly property int motionFlow: 800
    readonly property int motionPresence: 1200
    readonly property int motionRecall: 2000

    function coreAttention(state) {
        switch (state) {
        case AuroraTypes.CorePlaying: return 0.4
        case AuroraTypes.CoreGathering: return 0.35
        case AuroraTypes.CoreRecall: return 0.7
        case AuroraTypes.CoreTransition: return 0.8
        default: return 0.1
        }
    }

    function coreAccent(state) {
        switch (state) {
        case AuroraTypes.CorePlaying: return warmAccent
        case AuroraTypes.CorePaused: return coolAccent
        case AuroraTypes.CoreGathering: return memoryAccent
        case AuroraTypes.CoreTransition: return warmAccent
        case AuroraTypes.CoreRecall: return memoryAccent
        default: return textMuted
        }
    }

    function corePresence(state) {
        switch (state) {
        case AuroraTypes.CorePlaying: return 0.78
        case AuroraTypes.CorePaused: return 0.42
        case AuroraTypes.CoreGathering: return 0.60
        case AuroraTypes.CoreTransition: return 0.86
        case AuroraTypes.CoreRecall: return 0.72
        default: return 0.28
        }
    }

    function momentAccent(state) {
        switch (state) {
        case AuroraTypes.MomentRemembered: return coolAccent
        case AuroraTypes.MomentMeaningful: return memoryAccent
        case AuroraTypes.MomentRecalling: return warmAccent
        case AuroraTypes.MomentDetached: return detachedAccent
        case AuroraTypes.MomentArchived: return textMuted
        default: return coolAccent
        }
    }

    function momentPresence(state) {
        switch (state) {
        case AuroraTypes.MomentDormant: return 0.10
        case AuroraTypes.MomentPresent: return 0.22
        case AuroraTypes.MomentRemembered: return 0.32
        case AuroraTypes.MomentMeaningful: return 0.38
        case AuroraTypes.MomentRecalling: return 0.62
        case AuroraTypes.MomentArchived: return 0.08
        case AuroraTypes.MomentDetached: return 0.12
        default: return 0.18
        }
    }

    function momentOpacity(state) {
        switch (state) {
        case AuroraTypes.MomentDormant: return 0.66
        case AuroraTypes.MomentArchived: return 0.54
        case AuroraTypes.MomentDetached: return 0.74
        default: return 1.0
        }
    }

    function crystalOpacity(state) {
        switch (state) {
        case AuroraTypes.CrystalDormant: return 0.58
        case AuroraTypes.CrystalDetached: return 0.70
        case AuroraTypes.CrystalUnavailable: return 0.82
        default: return 1.0
        }
    }

    function momentSize(sizePreset) {
        switch (sizePreset) {
        case 0: return 240
        case 2: return 640
        default: return 400
        }
    }

    function momentRadius(sizePreset) {
        return Math.round(momentSize(sizePreset) * 0.16)
    }
}
