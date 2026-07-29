import QtQuick
import QtQuick.Dialogs
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced
    property bool identityVisible: true
    property bool transitioning: false
    property bool transitionLanding: false
    property bool diagnosticsVisible: false
    property bool flowSceneEnabled: true
    property bool libraryMenuExpanded: false
    property bool tracksPanelExpanded: false
    property bool sourceTracksMode: true
    property bool sourcePanelExpanded: false
    property bool controlMode: false
    property bool momentFeedbackVisible: false
    property bool appendResolvedSource: false
    property bool transitionCrystalSettling: false
    property bool presentationMode: false
    property bool networkTransitionEnabled: true
    property bool transitionLayoutControlsVisible: false
    property var activeNetworkTransitionAction: null
    property var pendingNetworkTransitionAction: null
    property bool transitionPlaybackRequested: false
    property bool resolvingNetworkTransitionTarget: false
    property string transitionTargetUrl: ""
    property real presencePhase: 0.0
    property int artworkRefreshAttempt: 0

    readonly property url observedArtworkSource:
        AudioRuntime.artworkSource
    readonly property string observedArtworkIdentity:
        AudioRuntime.trackId.length > 0
        ? AudioRuntime.trackId
        : root.displayTitle + "|" + root.displayArtist
    readonly property string observedArtworkRequestKey:
        observedArtworkSource.toString()
        + "|" + observedArtworkIdentity

    readonly property bool compactViewport: width < 1080 || height < 700
    readonly property bool cinematicWide: width >= 920
    readonly property bool performanceConstrained:
        root.qualityMode === AuroraTypes.Eco || root.compactViewport
    readonly property real presenceStrength:
        Math.max(0.0, Math.min(1.0,
            0.26
            + (AudioRuntime.playing ? 0.22 : AudioRuntime.hasTrack ? 0.10 : 0.0)
            + AudioRuntime.audioLevel * 0.24
            + AudioRuntime.bassEnergy * 0.18
            + AudioRuntime.transientEnergy * 0.10))

    readonly property string displayTitle:
        AudioRuntime.hasTrack ? AudioRuntime.title : AuroraI18n.text("demo.track")
    readonly property string displayArtist:
        AudioRuntime.hasTrack ? AudioRuntime.artist : AuroraI18n.text("demo.artist")
    readonly property string displayArtistLine:
        root.displayArtist === "Local audio" || root.displayArtist === "Online source"
        ? ""
        : root.displayArtist
    readonly property string displayAlbum: AudioRuntime.hasTrack ? AudioRuntime.album : ""
    readonly property color displayIdentityColor:
        AudioRuntime.hasTrack && AudioRuntime.identityColorAvailable
        ? AudioRuntime.identityColor
        : AuroraTokens.coolAccent
    property string transitionIncomingTitle: displayTitle
    property string transitionIncomingArtist: displayArtist
    property url transitionIncomingArtworkSource:
        AudioRuntime.hasTrack
        ? AudioRuntime.artworkSource
        : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
    property color transitionIncomingColor: displayIdentityColor
    readonly property real progressRatio:
        AudioRuntime.duration > 0 ? Math.min(1, AudioRuntime.position / AudioRuntime.duration) : 0
    readonly property bool libraryBrowserAvailable: LocalLibrary.sourceCount > 0 || LocalLibrary.searchText.length > 0
    readonly property int visiblePanelTrackCount:
        root.sourceTracksMode ? MusicSources.onlineTrackCount : LocalLibrary.visibleTrackCount
    readonly property bool forceControlsVisible: false
    readonly property bool controlsVisible:
        root.forceControlsVisible
        || root.controlMode
        || root.libraryMenuExpanded
        || root.tracksPanelExpanded
        || root.sourcePanelExpanded
    readonly property bool layoutControlsVisible:
        networkTransition.running
        ? root.transitionLayoutControlsVisible
        : root.controlsVisible
    readonly property real controlOpacity: root.controlsVisible ? 1.0 : 0.0
    readonly property bool reducedMotion:
        root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
    readonly property real visualPresence:
        Math.max(0.0, Math.min(1.0,
            (AudioRuntime.hasTrack ? 0.38 : 0.18)
            + (AudioRuntime.playing ? 0.18 : 0.0)
            + AudioRuntime.audioLevel * 0.28
            + AudioRuntime.bassEnergy * 0.12))
    readonly property bool emptyMusicPromptVisible:
        !AudioRuntime.hasTrack
        && LocalLibrary.trackCount === 0
        && !root.controlsVisible
    readonly property string firstRunLibraryMessage:
        LocalLibrary.defaultMusicDirectoryAvailable
        ? AuroraI18n.text("music.firstRunMusic")
        : AuroraI18n.text("music.firstRunChoose")
    readonly property bool currentMomentKeepable:
        AudioRuntime.hasTrack
        && AudioRuntime.trackId.length > 0
        && AudioRuntime.source.toString().length > 0
    readonly property string sourcePanelStatus:
        MusicSources.busy || MusicSources.resolving
        ? AuroraI18n.text("music.sourceStatusWorking")
        : MusicSources.errorString.length > 0
          ? AuroraI18n.text("music.sourceStatusFailure")
          : MusicSources.sourceCount > 0
            ? AuroraI18n.text("music.sourceStatusReady")
              + " · " + MusicSources.sourceCount
              + " · " + MusicSources.onlineTrackCount
              + " " + AuroraI18n.text("music.onlineTracksReady")
            : AuroraI18n.text("music.sourceStatusEmpty")
    readonly property string playbackTrustLabel:
        AudioRuntime.source.toString().startsWith("http://")
        || AudioRuntime.source.toString().startsWith("https://")
        ? AuroraI18n.text("music.onlineBadge")
        : AuroraI18n.text("music.localBadge")
    readonly property color mangaButtonFill: AuroraTokens.mangaPanel
    readonly property color mangaButtonBorder: AuroraTokens.mangaInk
    readonly property color mangaText: AuroraTokens.mangaInk
    readonly property color mangaMutedText: AuroraTokens.mangaMuted

    signal closeRequested()

    function clamp01(value) {
        return Math.max(0.0, Math.min(1.0, value))
    }

    function transitionLayerAmount(start, end) {
        if (!networkTransition.running)
            return 1.0
        const amount = root.clamp01((networkTransition.revealProgress - start)
                                    / Math.max(0.001, end - start))
        if (networkTransition.transitionState === "PlaybackRevealing")
            return 0.20 + amount * 0.80
        return amount
    }

    function transitionTrackKey(track, fallbackIndex) {
        if (!track)
            return "track:" + fallbackIndex
        if (track.sourceId && track.sourceId.length > 0)
            return track.sourceId
        if (track.cacheCatalogKey && track.cacheCatalogKey.length > 0)
            return track.cacheCatalogKey
        if (track.source && track.songId)
            return track.source + ":" + track.songId
        if (track.url && track.url.length > 0)
            return track.url
        if (track.trackId && track.trackId.length > 0)
            return track.trackId
        return "track:" + fallbackIndex + ":" + (track.title || "")
    }

    function currentNetworkTrack() {
        if (AudioRuntime.hasTrack && AudioRuntime.currentIndex >= 0)
            return AudioRuntime.queueTrackMetadata(AudioRuntime.currentIndex)

        return {
            title: root.displayTitle,
            artist: root.displayArtist,
            album: root.displayAlbum,
            url: AudioRuntime.source.toString(),
            trackId: AudioRuntime.trackId,
            sourceId: AudioRuntime.sourceId
        }
    }

    function queueNetworkTracks(targetTrack) {
        const tracks = []
        const seen = {}

        function append(track, index) {
            if (!track)
                return
            const key = root.transitionTrackKey(track, index)
            if (seen[key])
                return
            seen[key] = true
            tracks.push(track)
        }

        const count = AudioRuntime.queueCount
        if (count > 0) {
            const limit = Math.min(22, count)
            const current = Math.max(0, AudioRuntime.currentIndex)
            const start = current - Math.floor(limit / 2)
            for (let offset = 0; offset < limit; ++offset) {
                const index = (start + offset + count) % count
                append(AudioRuntime.queueTrackMetadata(index), index)
            }
        }

        append(root.currentNetworkTrack(), -1)
        append(targetTrack, -2)
        return tracks
    }

    function identityAnchorRect() {
        const anchorX = illustrationSpace.width * 0.34
        const anchorY = illustrationSpace.height * 0.20
        const anchorWidth = illustrationSpace.width * 0.42
        const anchorHeight = illustrationSpace.height * 0.60
        const origin = illustrationSpace.mapToItem(root, anchorX, anchorY)
        return Qt.rect(origin.x, origin.y, anchorWidth, anchorHeight)
    }

    function formatTime(milliseconds) {
        if (!isFinite(milliseconds) || milliseconds < 0)
            return "0:00"
        const totalSeconds = Math.floor(milliseconds / 1000)
        const minutes = Math.floor(totalSeconds / 60)
        const seconds = totalSeconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    function keepCurrentMoment() {
        if (!root.currentMomentKeepable)
            return

        if (Moments.keepCurrentMoment(
            AudioRuntime.trackId,
            AudioRuntime.sourceId,
            AudioRuntime.source,
            AudioRuntime.title,
            AudioRuntime.artist,
            AudioRuntime.album,
            AudioRuntime.artworkSource,
            root.displayIdentityColor,
            "")) {
            root.momentFeedbackVisible = true
            momentFeedbackTimer.restart()
        }
    }

    function sourceInputLooksLikeResolver(text) {
        return /^[A-Za-z]{2,4}\s*[:/, ]\s*[A-Za-z0-9_.-]+(?:\s*[:/, ]\s*[A-Za-z0-9]+)?$/.test(text.trim())
    }

    function sourceInputLooksLikeScript(text) {
        return /https?:\/\/\S+\.js(?:[?#]\S*)?/i.test(text)
               || (text.indexOf("@name") >= 0 && text.indexOf("EVENT_NAMES") >= 0)
    }

    function revealControls() {
        root.controlMode = true
        if (!root.libraryMenuExpanded
                && !root.tracksPanelExpanded
                && !root.sourcePanelExpanded)
            controlHideTimer.restart()
    }

    function playSourceInput(append) {
        const text = sourceInput.text.trim()
        if (text.length === 0)
            return

        if (!append && root.sourceInputLooksLikeScript(text)) {
            root.importSourceInput()
            return
        }

        if (root.sourceInputLooksLikeResolver(text)) {
            root.resolveSourceInput(append)
            return
        }

        if (append)
            AudioRuntime.appendSourcesFromText(text)
        else
            AudioRuntime.setQueueFromText(text)

        sourceInput.focus = false
        root.sourcePanelExpanded = false
    }

    function importSourceInput() {
        const text = sourceInput.text.trim()
        if (text.length === 0)
            return

        MusicSources.importFromText(text)
        sourceInput.text = ""
        sourceInput.focus = false
    }

    function resolveSourceInput(append) {
        const text = sourceInput.text.trim()
        root.appendResolvedSource = append === true
        if (text.length === 0)
            MusicSources.resolveDemoTrack()
        else
            MusicSources.resolveFromText(text)

        sourceInput.focus = false
    }

    function playOnlineTrack(index) {
        requestOnlineTrackTransition(index)
        sourceInput.focus = false
    }

    function beginTrackTransition(direction) {
        if (!root.networkTransitionEnabled) {
            if (direction > 0)
                AudioRuntime.next()
            else
                AudioRuntime.previous()
            return
        }

        if (AudioRuntime.queueCount < 2)
            return

        const targetIndex = AudioRuntime.wrappedQueueIndex(direction)
        if (targetIndex < 0)
            return

        const targetTrack = AudioRuntime.queueTrackMetadata(targetIndex)
        if (targetTrack.url && targetTrack.url.length > 0)
            AudioRuntime.warmQueueIndex(targetIndex)
        Lyrics.warmForTracks([targetTrack])

        root.requestNetworkTransition({
            kind: "direction",
            direction: direction < 0 ? -1 : 1,
            targetIndex: targetIndex,
            targetTrack: targetTrack
        })
    }

    function requestLocalTrackTransition(index, targetTrack) {
        if (!root.networkTransitionEnabled || !AudioRuntime.hasTrack) {
            AudioRuntime.setQueue(LocalLibrary.visiblePlayableUrlsStartingAt(index))
            root.tracksPanelExpanded = false
            return
        }

        root.requestNetworkTransition({
            kind: "localQueue",
            row: index,
            targetTrack
        })
        root.tracksPanelExpanded = false
    }

    function requestOnlineTrackTransition(index) {
        const tracks = MusicSources.onlineTracks
        const targetTrack = index >= 0 && index < tracks.length ? tracks[index] : ({})

        if (!root.networkTransitionEnabled || !AudioRuntime.hasTrack) {
            MusicSources.resolveOnlineTracksFrom(index)
            return
        }

        root.requestNetworkTransition({
            kind: "onlineCatalog",
            row: index,
            targetTrack
        })
    }

    function requestNetworkTransition(action) {
        if (!action || !action.targetTrack) {
            root.executeNetworkTransitionAction(action)
            return
        }

        if (networkTransition.running) {
            root.pendingNetworkTransitionAction = action
            return
        }

        const currentTrack = root.currentNetworkTrack()
        const targetTrack = action.targetTrack
        root.activeNetworkTransitionAction = action
        root.pendingNetworkTransitionAction = null
        root.transitionPlaybackRequested = false
        root.resolvingNetworkTransitionTarget = false
        root.transitionTargetUrl = targetTrack.url || ""
        root.transitionLayoutControlsVisible = root.controlsVisible

        root.transitionIncomingTitle =
                targetTrack.title && targetTrack.title.length > 0
                ? targetTrack.title
                : root.displayTitle
        root.transitionIncomingArtist =
                targetTrack.artist && targetTrack.artist.length > 0
                ? targetTrack.artist
                : root.displayArtist
        root.transitionIncomingArtworkSource =
                targetTrack.artworkUrl && targetTrack.artworkUrl.length > 0
                ? targetTrack.artworkUrl
                : "qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"
        root.transitionIncomingColor =
                targetTrack.identityColor && targetTrack.identityColor.length > 0
                ? targetTrack.identityColor
                : root.displayIdentityColor

        root.transitionLanding = false
        root.transitionCrystalSettling = true
        networkTransition.startTransition(
                    currentTrack,
                    targetTrack,
                    root.queueNetworkTracks(targetTrack))
    }

    function executeNetworkTransitionAction(action) {
        if (!action)
            return

        root.transitionPlaybackRequested = true
        root.activeNetworkTransitionAction = action
        root.transitionTargetUrl = action.targetTrack && action.targetTrack.url
                ? action.targetTrack.url
                : ""

        if (action.kind === "direction") {
            AudioRuntime.beginDeferredQueueStep(action.direction)
            root.transitionTargetUrl = AudioRuntime.source.toString()
            AudioRuntime.commitDeferredPlayback(true)
            playbackStartPollTimer.restart()
            return
        }

        if (action.kind === "localQueue") {
            AudioRuntime.setQueue(LocalLibrary.visiblePlayableUrlsStartingAt(action.row))
            playbackStartPollTimer.restart()
            return
        }

        if (action.kind === "onlineCatalog") {
            root.resolvingNetworkTransitionTarget = true
            MusicSources.resolveOnlineTracksFrom(action.row)
        }
    }

    function networkTargetPlaybackStarted() {
        if (!networkTransition.running
                || networkTransition.transitionState !== "WaitingForAudio"
                || !root.transitionPlaybackRequested
                || root.resolvingNetworkTransitionTarget) {
            return false
        }

        if (root.transitionTargetUrl.length > 0
                && AudioRuntime.source.toString() !== root.transitionTargetUrl) {
            return false
        }

        return AudioRuntime.playing
    }

    function checkNetworkPlaybackStarted() {
        if (root.networkTargetPlaybackStarted()) {
            playbackStartPollTimer.stop()
            networkTransition.confirmPlaybackStarted()
        }
    }

    function resetNetworkTransitionRuntime() {
        playbackStartPollTimer.stop()
        root.activeNetworkTransitionAction = null
        root.transitionPlaybackRequested = false
        root.resolvingNetworkTransitionTarget = false
        root.transitionTargetUrl = ""
    }


    // AUR-CONTROLS-REFERENCE-PACK-04:BEGIN
    component MangekyoEyeButton: Item {
        id: eyeButton

        property int direction: -1
        property bool reducedMotion: false
        property bool armed: true
        property real blinkProgress: 0.0
        property real breathingPhase: 0.0
        property real hoverAmount: hoverHandler.hovered ? 1.0 : 0.0
        property real pressAmount: tapHandler.pressed ? 1.0 : 0.0

        signal activated()

        width: 112
        height: 72
        opacity: armed ? 1.0 : 0.44
        scale: 1.0 + hoverAmount * 0.035 - pressAmount * 0.025

        function blinkAndActivate() {
            if (!armed || clickBlink.running)
                return
            clickBlink.restart()
        }

        Timer {
            id: idleBlinkTimer

            interval: 2600
            repeat: true
            running: eyeButton.visible

            onTriggered: {
                if (!idleBlink.running && !clickBlink.running)
                    idleBlink.restart()
                interval = 2300 + Math.floor(Math.random() * 2100)
            }
        }

        SequentialAnimation {
            id: idleBlink

            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 1.0
                duration: 105
                easing.type: Easing.InCubic
            }

            PauseAnimation { duration: 44 }

            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 185
                easing.type: Easing.OutCubic
            }
        }

        SequentialAnimation {
            id: clickBlink

            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 1.0
                duration: 82
                easing.type: Easing.InCubic
            }

            PauseAnimation { duration: 36 }

            ScriptAction {
                script: eyeButton.activated()
            }

            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 190
                easing.type: Easing.OutBack
            }
        }

        NumberAnimation on breathingPhase {
            from: 0.0
            to: Math.PI * 2.0
            duration: 3600
            loops: Animation.Infinite
            running: eyeButton.visible && !eyeButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation {
                duration: 130
                easing.type: Easing.OutCubic
            }
        }

        HoverHandler {
            id: hoverHandler
        }

        TapHandler {
            id: tapHandler

            enabled: eyeButton.armed
            onTapped: eyeButton.blinkAndActivate()
        }

        Item {
            id: eyeViewport

            anchors.fill: parent
            clip: false

            Image {
                id: eyeImage

                anchors.centerIn: parent
                width: parent.width
                height: parent.height * 0.88
                source:
                    "qrc:/qt/qml/Aurora/App/assets/controls/mangekyo-eye-reference.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
                asynchronous: false
                cache: true
                mirror: eyeButton.direction > 0
                opacity: 1.0 - Math.max(
                    0.0,
                    (eyeButton.blinkProgress - 0.84) / 0.16)
                scale:
                    1.0
                    + Math.sin(eyeButton.breathingPhase) * 0.008
                    + eyeButton.hoverAmount * 0.018

                transform: Scale {
                    origin.x: eyeImage.width * 0.5
                    origin.y: eyeImage.height * 0.5
                    xScale: 1.0
                    yScale: Math.max(
                        0.035,
                        1.0 - eyeButton.blinkProgress * 0.965)
                }

                Behavior on scale {
                    NumberAnimation {
                        duration: 130
                        easing.type: Easing.OutCubic
                    }
                }
            }

            Canvas {
                id: closedLidCanvas

                anchors.fill: parent
                opacity: Math.max(
                    0.0,
                    (eyeButton.blinkProgress - 0.68) / 0.32)
                antialiasing: true
                renderTarget: Canvas.Image

                onOpacityChanged: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()

                onPaint: {
                    const ctx = getContext("2d")
                    const w = width
                    const h = height
                    ctx.clearRect(0, 0, w, h)

                    const cy = h * 0.48
                    const left = w * 0.09
                    const right = w * 0.91

                    ctx.beginPath()
                    ctx.moveTo(left, cy)
                    ctx.bezierCurveTo(
                        w * 0.30,
                        cy + h * 0.08,
                        w * 0.70,
                        cy + h * 0.08,
                        right,
                        cy)
                    ctx.lineWidth = 3.4
                    ctx.lineCap = "round"
                    ctx.strokeStyle = "rgba(30,20,22,0.94)"
                    ctx.stroke()

                    ctx.beginPath()
                    ctx.moveTo(left + w * 0.04, cy - h * 0.02)
                    ctx.bezierCurveTo(
                        w * 0.32,
                        cy - h * 0.07,
                        w * 0.68,
                        cy - h * 0.07,
                        right - w * 0.04,
                        cy - h * 0.02)
                    ctx.lineWidth = 1.2
                    ctx.strokeStyle = "rgba(108,58,56,0.38)"
                    ctx.stroke()
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -4
                text: eyeButton.direction < 0 ? "‹" : "›"
                color: Qt.rgba(26 / 255.0, 20 / 255.0, 23 / 255.0, 0.48)
                font.pixelSize: 18
                font.weight: Font.Medium
            }
        }
    }

    component LamborghiniPlayToggle: Item {
        id: carButton

        property bool playing: false
        property bool hasTrack: false
        property bool reducedMotion: false
        property real drivePhase: 0.0
        property real wheelPhase: 0.0
        property real suspensionPhase: 0.0
        property real hoverAmount: carHover.hovered ? 1.0 : 0.0
        property real pressAmount: carTap.pressed ? 1.0 : 0.0

        signal activated()

        width: 204
        height: 92
        opacity: hasTrack ? 1.0 : 0.66
        scale: 1.0 + hoverAmount * 0.03 - pressAmount * 0.02

        NumberAnimation on drivePhase {
            from: 0.0
            to: 1.0
            duration: 720
            loops: Animation.Infinite
            running: carButton.playing && !carButton.reducedMotion
        }

        NumberAnimation on wheelPhase {
            from: 0.0
            to: 360.0
            duration: 560
            loops: Animation.Infinite
            running: carButton.playing && !carButton.reducedMotion
        }

        NumberAnimation on suspensionPhase {
            from: -1.0
            to: 1.0
            duration: 980
            loops: Animation.Infinite
            easing.type: Easing.InOutSine
            running: carButton.playing && !carButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation {
                duration: 130
                easing.type: Easing.OutCubic
            }
        }

        HoverHandler {
            id: carHover
        }

        TapHandler {
            id: carTap
            onTapped: carButton.activated()
        }

        Canvas {
            id: motionCanvas

            anchors.fill: parent
            antialiasing: true
            renderTarget: Canvas.Image

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                if (!carButton.playing)
                    return

                for (let index = 0; index < 5; ++index) {
                    const phase =
                        (carButton.drivePhase + index * 0.19) % 1.0
                    const y = h * (0.58 + index * 0.055)
                    const startX = w * (0.20 + phase * 0.26)
                    const length = w * (0.075 + index * 0.014)

                    ctx.beginPath()
                    ctx.moveTo(startX, y)
                    ctx.lineTo(startX - length, y)
                    ctx.lineWidth = 1.5
                    ctx.strokeStyle = "rgba(35,31,34,0.22)"
                    ctx.stroke()
                }

                const flare =
                    0.5
                    + Math.sin(carButton.drivePhase * Math.PI * 2.0)
                      * 0.16

                ctx.beginPath()
                ctx.moveTo(w * 0.90, h * 0.58)
                ctx.lineTo(w * (0.96 + flare * 0.025), h * 0.54)
                ctx.lineTo(w * (0.95 + flare * 0.024), h * 0.63)
                ctx.closePath()
                ctx.fillStyle = "rgba(78,129,255,0.26)"
                ctx.fill()
            }
        }

        Image {
            id: carImage

            width: parent.width
            height: parent.height
            x: (parent.width - width) * 0.5
            source:
                "qrc:/qt/qml/Aurora/App/assets/controls/lamborghini-reference.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
            asynchronous: false
            cache: true
            y:
                (parent.height - height) * 0.5
                + (carButton.playing
                   ? carButton.suspensionPhase * 1.8
                   : 0.0)
            scale:
                1.0
                + carButton.hoverAmount * 0.02
                + (carButton.playing
                   ? Math.sin(carButton.drivePhase * Math.PI * 2.0)
                     * 0.006
                   : 0.0)
        }

        Canvas {
            id: wheelsCanvas

            anchors.fill: parent
            antialiasing: true
            renderTarget: Canvas.Image

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                const wheels = [
                    { x: w * 0.164, y: h * 0.63, r: h * 0.105 },
                    { x: w * 0.612, y: h * 0.63, r: h * 0.112 }
                ]

                for (let index = 0; index < wheels.length; ++index) {
                    const wheel = wheels[index]

                    ctx.save()
                    ctx.translate(wheel.x, wheel.y)
                    ctx.rotate(
                        carButton.wheelPhase
                        * Math.PI / 180.0)

                    ctx.lineWidth = 1.15
                    ctx.strokeStyle =
                        carButton.playing
                        ? "rgba(235,240,245,0.72)"
                        : "rgba(235,240,245,0.28)"

                    for (let spoke = 0; spoke < 7; ++spoke) {
                        ctx.rotate(Math.PI * 2.0 / 7.0)
                        ctx.beginPath()
                        ctx.moveTo(0, -wheel.r * 0.18)
                        ctx.lineTo(0, -wheel.r * 0.58)
                        ctx.stroke()
                    }

                    ctx.restore()
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: -3
            visible: !carButton.playing
            text: "Ⅱ"
            color: Qt.rgba(25 / 255.0, 21 / 255.0, 23 / 255.0, 0.48)
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        onDrivePhaseChanged: {
            motionCanvas.requestPaint()
            wheelsCanvas.requestPaint()
        }

        onWheelPhaseChanged: wheelsCanvas.requestPaint()
        onPlayingChanged: {
            motionCanvas.requestPaint()
            wheelsCanvas.requestPaint()
        }
    }
    // AUR-CONTROLS-REFERENCE-PACK-04:END

    NumberAnimation on presencePhase {
        from: 0.0
        to: Math.PI * 2.0
        duration: root.qualityMode === AuroraTypes.Immersive ? 15000 : 21000
        loops: Animation.Infinite
        running: root.visible
                 && !root.reducedMotion
                 && root.qualityMode !== AuroraTypes.Eco
    }

    focus: visible
    Keys.onSpacePressed: {
        root.revealControls()
        AudioRuntime.togglePlayback()
    }
    Keys.onLeftPressed: event => {
        root.revealControls()
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(-1)
        else
            AudioRuntime.seekRelative(-5000)
        event.accepted = true
    }
    Keys.onRightPressed: event => {
        root.revealControls()
        if (event.modifiers & Qt.ShiftModifier)
            root.beginTrackTransition(1)
        else
            AudioRuntime.seekRelative(5000)
        event.accepted = true
    }
    Keys.onPressed: event => {
        root.revealControls()
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_O) {
            audioDialog.open()
            event.accepted = true
            return
        }
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_L) {
            libraryDialog.open()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_K
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            root.keepCurrentMoment()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_D
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_V
                && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
            root.flowSceneEnabled = !root.flowSceneEnabled
            event.accepted = true
        }
    }

    onVisibleChanged: {
        if (visible) {
            MusicSources.loadOnlineTracks()
            root.controlMode = root.forceControlsVisible
            forceActiveFocus()
        }
    }

    onLibraryMenuExpandedChanged: {
        if (libraryMenuExpanded)
            root.revealControls()
        else if (!tracksPanelExpanded)
            controlHideTimer.restart()
    }

    onTracksPanelExpandedChanged: {
        if (tracksPanelExpanded)
            root.revealControls()
        else if (!libraryMenuExpanded && !sourcePanelExpanded)
            controlHideTimer.restart()
    }

    onSourcePanelExpandedChanged: {
        if (sourcePanelExpanded)
            root.revealControls()
        else if (!libraryMenuExpanded && !tracksPanelExpanded)
            controlHideTimer.restart()
    }

    Timer {
        id: controlHideTimer
        interval: root.presentationMode ? 2400 : 3200
        repeat: false
        onTriggered: {
            if (networkTransition.running) {
                controlHideTimer.restart()
                return
            }
            if (!root.forceControlsVisible
                    && !root.libraryMenuExpanded
                    && !root.tracksPanelExpanded
                    && !root.sourcePanelExpanded)
                root.controlMode = false
        }
    }

    Timer {
        id: momentFeedbackTimer
        interval: 1800
        repeat: false
        onTriggered: root.momentFeedbackVisible = false
    }

    Timer {
        id: crystalSettleTimer
        interval: 180
        repeat: false
        onTriggered: {
            root.transitionCrystalSettling = false
            root.queueArtworkIllustrationRefresh(false)
        }
    }

    Timer {
        id: playbackStartPollTimer
        interval: 120
        repeat: true
        onTriggered: root.checkNetworkPlaybackStarted()
    }

    FileDialog {
        id: audioDialog
        title: AuroraI18n.text("music.audioDialog")
        fileMode: FileDialog.OpenFiles
        nameFilters: [
            "Audio files (*.mp3 *.flac *.ogg *.opus *.wav *.m4a *.aac)",
            "Playlists (*.m3u *.m3u8)",
            "All files (*)"
        ]
        onAccepted: AudioRuntime.setQueue(selectedFiles)
    }

    FolderDialog {
        id: libraryDialog
        title: AuroraI18n.text("music.folderDialog")
        onAccepted: LocalLibrary.scanDirectory(selectedFolder)
    }

    Connections {
        target: MusicSources

        function onMusicUrlResolved(url) {
            if (root.appendResolvedSource)
                AudioRuntime.appendSourcesFromText(url)
            else
                AudioRuntime.setQueueFromText(url)
            root.appendResolvedSource = false
            root.sourcePanelExpanded = false
            root.tracksPanelExpanded = false
        }

        function onMusicUrlsResolved(urls) {
            if (root.appendResolvedSource)
                AudioRuntime.appendSourcesFromText(urls.join("\n"))
            else
                AudioRuntime.setQueueFromText(urls.join("\n"))
            root.appendResolvedSource = false
            root.sourcePanelExpanded = false
            root.tracksPanelExpanded = false
        }

        function onMusicTracksResolved(tracks) {
            Lyrics.rememberTracks(tracks)
            Lyrics.warmForTracks(tracks)
            AudioRuntime.setQueueWithMetadata(tracks)
            if (root.resolvingNetworkTransitionTarget) {
                root.resolvingNetworkTransitionTarget = false
                if (tracks.length > 0) {
                    root.transitionTargetUrl = tracks[0].url
                    playbackStartPollTimer.restart()
                } else {
                    networkTransition.cancelToCurrent()
                    root.resetNetworkTransitionRuntime()
                }
            }
            root.appendResolvedSource = false
            root.sourcePanelExpanded = false
            root.tracksPanelExpanded = false
        }

        function onMusicTracksAppendResolved(tracks) {
            Lyrics.rememberTracks(tracks)
            Lyrics.warmForTracks(tracks)
            AudioRuntime.appendQueueWithMetadata(tracks)
            root.appendResolvedSource = false
        }
    }

    function queueArtworkIllustrationRefresh(resetAttempts) {
        if (resetAttempts)
            root.artworkRefreshAttempt = 0
        const deferForTransition =
                networkTransition.running
                && networkTransition.transitionState !== "WaitingForAudio"
                && networkTransition.transitionState !== "PlaybackRevealing"
        artworkRefreshTimer.interval =
                deferForTransition
                ? 280
                : (networkTransition.running ? 30 : 90)
        artworkRefreshTimer.restart()
    }

    function refreshArtworkIllustration() {
        const deferForTransition =
                networkTransition.running
                && networkTransition.transitionState !== "WaitingForAudio"
                && networkTransition.transitionState !== "PlaybackRevealing"

        if (deferForTransition) {
            artworkRefreshTimer.interval = 280
            artworkRefreshTimer.restart()
            return
        }

        const source = root.observedArtworkSource
        const identity = root.observedArtworkIdentity

        if (source.toString().length === 0) {
            ArtworkIllustration.clear()
            return
        }

        ArtworkIllustration.load(source, identity)
    }

    function scheduleArtworkRetry() {
        if (root.observedArtworkSource.toString().length === 0)
            return
        if (ArtworkIllustration.ready || ArtworkIllustration.loading)
            return
        // AUR-SOURCE-ARTWORK-RECOVERY-PACK-01:QML-RETRY
        if (root.artworkRefreshAttempt >= 5)
            return;

        root.artworkRefreshAttempt += 1
        artworkRetryTimer.interval =
                root.artworkRefreshAttempt === 1 ? 280
                : root.artworkRefreshAttempt === 2 ? 900
                : root.artworkRefreshAttempt === 3 ? 1800
                : root.artworkRefreshAttempt === 4 ? 3500
                : 6500
        artworkRetryTimer.restart()
    }

    Timer {
        id: artworkRefreshTimer

        interval: 90
        repeat: false
        onTriggered: root.refreshArtworkIllustration()
    }

    Timer {
        id: artworkRetryTimer

        interval: 280
        repeat: false
        onTriggered: root.refreshArtworkIllustration()
    }

    onObservedArtworkRequestKeyChanged: {
        ArtworkIllustration.clear()
        root.queueArtworkIllustrationRefresh(true)
    }

    Connections {
        target: AudioRuntime

        function onSourceChanged() {
            Lyrics.loadForSource(AudioRuntime.source)
            Lyrics.setPosition(AudioRuntime.position)
            root.queueArtworkIllustrationRefresh(false)
            root.checkNetworkPlaybackStarted()
        }

        function onTrackChanged() {
            if (networkTransition.running
                    && !root.transitionPlaybackRequested
                    && networkTransition.transitionState !== "Idle") {
                networkTransition.cancelToCurrent()
                root.resetNetworkTransitionRuntime()
            }
            root.queueArtworkIllustrationRefresh(true)
        }

        function onPositionChanged() {
            Lyrics.setPosition(AudioRuntime.position)
            root.checkNetworkPlaybackStarted()
        }

        function onPlaybackStateChanged() {
            root.checkNetworkPlaybackStarted()
        }

        function onErrorChanged() {
            if (networkTransition.running
                    && networkTransition.transitionState === "WaitingForAudio"
                    && AudioRuntime.errorString.length > 0) {
                networkTransition.cancelToCurrent()
                root.resetNetworkTransitionRuntime()
                root.transitionLanding = false
                root.transitionCrystalSettling = false
            }
        }
    }

    Connections {
        target: ArtworkIllustration

        function onStateChanged() {
            if (ArtworkIllustration.ready) {
                root.artworkRefreshAttempt = 0
                return
            }

            if (!ArtworkIllustration.loading
                && ArtworkIllustration.errorString.length > 0) {
                root.scheduleArtworkRetry()
            }
        }
    }

    Component.onCompleted: {
        Lyrics.loadForSource(AudioRuntime.source)
        Lyrics.setPosition(AudioRuntime.position)
        root.queueArtworkIllustrationRefresh(true)
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        z: 10
        onPositionChanged: root.revealControls()
    }

    MangaBackdrop {
        anchors.fill: parent
        opacity: ArtworkIllustration.ready
                 ? (root.controlsVisible ? 0.11 : 0.055)
                 : (root.controlsVisible ? 0.30 : 0.18)
        accentColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        energy: (AudioRuntime.playing ? 0.48 : AudioRuntime.hasTrack ? 0.30 : 0.16)
                + AudioRuntime.audioLevel * 0.52
                + AudioRuntime.transientEnergy * 0.22
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
    }

    AtmosphereField {
        anchors.fill: parent
        primaryColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.50 : AudioRuntime.hasTrack ? 0.34 : 0.22
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        paperMode: true
        opacity: root.flowSceneEnabled
                 ? 0.0
                 : (root.controlsVisible ? 0.08 : 0.14)

        Behavior on opacity {
            NumberAnimation {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    AuroraFlowScene {
        anchors.fill: parent
        primaryColor: root.displayIdentityColor
        secondaryColor: AuroraTokens.memoryAccent
        presenceLevel: AudioRuntime.playing ? 0.56 : AudioRuntime.hasTrack ? 0.38 : 0.22
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
        paperMode: true
        opacity: ArtworkIllustration.ready
                 ? (root.flowSceneEnabled ? 0.025 : 0.0)
                 : (root.flowSceneEnabled
                    ? (root.controlsVisible ? 0.08 : 0.12)
                    : 0.0)

        Behavior on opacity {
            NumberAnimation {
                duration: root.accessibilityMode === AuroraTypes.AccessibilityReducedMotion
                          ? AuroraTokens.motionSoft
                          : AuroraTokens.motionFlow
                easing.type: Easing.OutCubic
            }
        }
    }

    Item {
        id: cinematicColorField

        anchors.fill: parent
        opacity: ArtworkIllustration.ready
                 ? (root.controlsVisible ? 0.08 : 0.12)
                 : (root.controlsVisible ? 0.18 : 0.28)

        Repeater {
            model: 3

            delegate: Rectangle {
                required property int index

                width: parent.width * (index === 0 ? 0.56 : index === 1 ? 0.42 : 0.34)
                height: parent.height * (index === 0 ? 0.72 : index === 1 ? 0.58 : 0.46)
                radius: Math.min(width, height) * 0.48
                x: index === 0
                   ? parent.width * 0.42 + Math.sin(root.presencePhase * 0.23) * 24
                   : index === 1
                     ? parent.width * 0.04 + Math.sin(root.presencePhase * 0.31 + 1.2) * 18
                     : parent.width * 0.66 + Math.sin(root.presencePhase * 0.27 + 2.1) * 16
                y: index === 0
                   ? parent.height * 0.08 + Math.cos(root.presencePhase * 0.19) * 16
                   : index === 1
                     ? parent.height * 0.34 + Math.cos(root.presencePhase * 0.25 + 0.8) * 14
                     : parent.height * 0.44 + Math.cos(root.presencePhase * 0.21 + 1.6) * 12
                color: index === 0
                       ? Qt.rgba(root.displayIdentityColor.r,
                                 root.displayIdentityColor.g,
                                 root.displayIdentityColor.b,
                                 0.045 + root.presenceStrength * 0.055)
                       : index === 1
                         ? Qt.rgba(AuroraTokens.memoryAccent.r,
                                   AuroraTokens.memoryAccent.g,
                                   AuroraTokens.memoryAccent.b,
                                   0.032 + AudioRuntime.midEnergy * 0.045)
                         : Qt.rgba(AuroraTokens.warmAccent.r,
                                   AuroraTokens.warmAccent.g,
                                   AuroraTokens.warmAccent.b,
                                   0.026 + AudioRuntime.highEnergy * 0.038)
                rotation: root.reducedMotion
                          ? (index === 0 ? -8 : index === 1 ? 11 : -4)
                          : (index === 0 ? -8 : index === 1 ? 11 : -4)
                            + Math.sin(root.presencePhase * (0.18 + index * 0.04)) * 2.2
                scale: 1.0
                       + AudioRuntime.bassEnergy * (0.025 + index * 0.010)
                       + AudioRuntime.transientEnergy * 0.012

                Behavior on scale {
                    NumberAnimation { duration: 240; easing.type: Easing.OutCubic }
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 420; easing.type: Easing.OutCubic }
        }
    }

    Item {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -34
        width: Math.min(root.width * 0.58, root.height * 0.78)
        height: width * 0.86
        opacity: AudioRuntime.hasTrack ? (root.controlsVisible ? 0.22 : 0.30) : 0.16
        scale: 1.0 + AudioRuntime.bassEnergy * 0.035 + AudioRuntime.transientEnergy * 0.020

        Rectangle {
            anchors.fill: parent
            radius: Math.min(width, height) * 0.22
            color: Qt.rgba(root.displayIdentityColor.r,
                           root.displayIdentityColor.g,
                           root.displayIdentityColor.b,
                           0.014 + AudioRuntime.audioLevel * 0.018)
            border.width: 3
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.06 + AudioRuntime.highEnergy * 0.06)
            rotation: -8 + AudioRuntime.midEnergy * 3
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.74
            height: parent.height * 0.62
            radius: Math.min(width, height) * 0.24
            color: Qt.rgba(AuroraTokens.memoryAccent.r,
                           AuroraTokens.memoryAccent.g,
                           AuroraTokens.memoryAccent.b,
                           0.012 + AudioRuntime.highEnergy * 0.022)
            border.width: 2
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.05 + AudioRuntime.transientEnergy * 0.07)
            rotation: 11 - AudioRuntime.highEnergy * 4
        }

        Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: 420; easing.type: Easing.OutCubic } }
    }

    Item {
        id: presenceAura

        anchors.centerIn: parent
        anchors.verticalCenterOffset: root.controlsVisible ? -54 : -68
        width: Math.min(root.width * 0.76, root.height * 1.02)
        height: width
        opacity: AudioRuntime.hasTrack
                 ? (root.controlsVisible ? 0.28 : 0.48)
                 : 0.16
        scale: 1.0
               + AudioRuntime.bassEnergy * 0.032
               + AudioRuntime.transientEnergy * 0.018
        rotation: root.reducedMotion
                  ? 0
                  : Math.sin(root.presencePhase * 0.42) * 2.4
        Repeater {
            model: 2

            delegate: Rectangle {
                required property int index

                anchors.centerIn: parent
                width: parent.width * (0.58 + index * 0.13)
                height: width
                radius: width * (0.22 + index * 0.018)
                color: "transparent"
                border.width: index === 0 ? 2 : 1
                border.color: Qt.rgba(root.displayIdentityColor.r,
                                      root.displayIdentityColor.g,
                                      root.displayIdentityColor.b,
                                      0.10
                                      + root.visualPresence * (0.10 - index * 0.018)
                                      + AudioRuntime.highEnergy * 0.08)
                rotation: (index % 2 === 0 ? -1 : 1)
                          * (7 + index * 5)
                          + (root.reducedMotion
                             ? 0
                             : Math.sin(root.presencePhase * (0.34 + index * 0.07))
                               * (2.0 + index))
                scale: 1.0
                       + AudioRuntime.bassEnergy * (0.020 + index * 0.012)
                       + AudioRuntime.transientEnergy * (0.015 + index * 0.010)
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.48
            height: width
            radius: width / 2
            color: Qt.rgba(root.displayIdentityColor.r,
                           root.displayIdentityColor.g,
                           root.displayIdentityColor.b,
                           0.026 + root.visualPresence * 0.050)
            scale: 1.0 + AudioRuntime.bassEnergy * 0.080
        }

        Behavior on opacity {
            NumberAnimation { duration: 420; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.max(86, parent.height * 0.11)
        width: 178
        height: 46
        visible: root.emptyMusicPromptVisible
        opacity: root.emptyMusicPromptVisible ? 1.0 : 0.0
        radius: 9
        color: AuroraTokens.mangaPanel
        border.width: 3
        border.color: AuroraTokens.mangaInk
        rotation: 1.0
        z: 9

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.connectSource")
            color: AuroraTokens.mangaInk
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler {
            onTapped: {
                root.sourcePanelExpanded = true
                root.revealControls()
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 28
        width: 88
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.back")
            color: root.mangaText
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.closeRequested() }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 436
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.flowSceneEnabled
                      ? root.displayIdentityColor
                      : root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: root.flowSceneEnabled
                  ? AuroraI18n.text("music.sceneFlow")
                  : AuroraI18n.text("music.sceneField")
            color: root.mangaText
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: root.flowSceneEnabled = !root.flowSceneEnabled }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: 300
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 124
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.currentMomentKeepable
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.18)
               : AuroraTokens.mangaWash
        border.width: 2
        border.color: root.currentMomentKeepable
                      ? root.displayIdentityColor
                      : Qt.rgba(AuroraTokens.mangaInk.r,
                                AuroraTokens.mangaInk.g,
                                AuroraTokens.mangaInk.b,
                                0.38)

        Text {
            anchors.centerIn: parent
            text: Moments.latestTrackId === AudioRuntime.trackId
                  ? AuroraI18n.text("music.keepAgain")
                  : AuroraI18n.text("music.keepMoment")
            color: root.currentMomentKeepable
                   ? root.mangaText
                   : root.mangaMutedText
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler {
            enabled: root.currentMomentKeepable
            onTapped: root.keepCurrentMoment()
        }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 28
        width: Math.min(360, root.width - 56)
        height: 42
        visible: root.momentFeedbackVisible
                 || (root.controlsVisible && Moments.errorString.length > 0)
        opacity: visible ? 1.0 : 0.0
        scale: visible ? 1.0 : 0.96
        radius: 10
        color: AuroraTokens.mangaPanel
        border.width: 2
        border.color: Moments.errorString.length > 0
                      ? AuroraTokens.warmAccent
                      : root.displayIdentityColor
        z: 40

        Text {
            anchors.fill: parent
            anchors.margins: 10
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Moments.errorString.length > 0
                  ? Moments.errorString
                  : Moments.lastStatus
            color: Moments.errorString.length > 0
                   ? AuroraTokens.warmAccent
                   : root.mangaText
            font.pixelSize: 12
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Behavior on opacity { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        visible: false
        anchors.right: parent.right
        anchors.rightMargin: 172
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 116
        height: 42
        radius: 21
        color: root.diagnosticsVisible
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.18)
               : Qt.rgba(1, 1, 1, 0.07)
        border.width: 1
        border.color: AudioRuntime.audioReactiveAvailable
                      ? Qt.rgba(root.displayIdentityColor.r,
                                root.displayIdentityColor.g,
                                root.displayIdentityColor.b, 0.48)
                      : Qt.rgba(1, 1, 1, 0.10)

        Row {
            anchors.centerIn: parent
            spacing: 7

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 7
                height: 7
                radius: 4
                color: AudioRuntime.audioReactiveAvailable
                       ? (AudioRuntime.playing ? AuroraTokens.warmAccent : AuroraTokens.coolAccent)
                       : AuroraTokens.textMuted
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Audio data"
                color: AuroraTokens.textSecondary
                font.pixelSize: 12
            }
        }

        TapHandler { onTapped: root.diagnosticsVisible = !root.diagnosticsVisible }
    }

    Rectangle {
        id: sourceButton

        anchors.right: parent.right
        anchors.rightMargin: 174
        anchors.top: parent.top
        anchors.topMargin: 28
        width: 132
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 21
        color: root.sourcePanelExpanded
               ? Qt.rgba(root.displayIdentityColor.r,
                         root.displayIdentityColor.g,
                         root.displayIdentityColor.b, 0.20)
               : root.mangaButtonFill
        border.width: 2
        border.color: root.sourcePanelExpanded
                      ? root.displayIdentityColor
                      : root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AuroraI18n.text("music.source")
                  + (MusicSources.sourceCount > 0
                     ? " · " + MusicSources.sourceCount
                     : "")
            color: root.mangaText
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        TapHandler {
            onTapped: root.sourcePanelExpanded = !root.sourcePanelExpanded
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 320
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: 320
                easing.type: Easing.OutCubic
            }
        }
    }

    Rectangle {
        id: sourceStudioPanel

        anchors.right: parent.right
        anchors.rightMargin: root.compactViewport ? 18 : 28
        anchors.top: sourceButton.bottom
        anchors.topMargin: 10
        width: Math.min(root.compactViewport ? 430 : 486,
                        root.width - (root.compactViewport ? 36 : 56))
        height: Math.min(root.compactViewport ? 382 : 414,
                         root.height - (root.compactViewport ? 94 : 112))
        visible: root.sourcePanelExpanded
        enabled: root.sourcePanelExpanded
        opacity: root.sourcePanelExpanded ? 1.0 : 0.0
        scale: root.sourcePanelExpanded ? 1.0 : 0.975
        radius: 14
        color: Qt.rgba(AuroraTokens.mangaPanel.r,
                       AuroraTokens.mangaPanel.g,
                       AuroraTokens.mangaPanel.b,
                       0.97)
        border.width: 2
        border.color: AuroraTokens.mangaInk
        z: 24

        Behavior on opacity {
            NumberAnimation {
                duration: 240
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: 280
                easing.type: Easing.OutQuint
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 12
            anchors.topMargin: 12
            width: 30
            height: 30
            radius: 15
            color: Qt.rgba(AuroraTokens.mangaPaper.r,
                           AuroraTokens.mangaPaper.g,
                           AuroraTokens.mangaPaper.b,
                           0.72)
            border.width: 1
            border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                  AuroraTokens.mangaInk.g,
                                  AuroraTokens.mangaInk.b,
                                  0.22)
            z: 4

            Text {
                anchors.centerIn: parent
                text: "×"
                color: root.mangaText
                font.pixelSize: 18
            }

            TapHandler { onTapped: root.sourcePanelExpanded = false }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 11

            Row {
                width: parent.width
                height: 48
                spacing: 10

                Column {
                    width: parent.width - 156
                    spacing: 3

                    Text {
                        text: AuroraI18n.text("music.sourceStudio")
                        color: root.mangaText
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }

                    Text {
                        width: parent.width
                        text: AuroraI18n.text("music.sourceStudioHint")
                        color: root.mangaMutedText
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    width: 114
                    height: 34
                    radius: 17
                    color: Qt.rgba(root.displayIdentityColor.r,
                                   root.displayIdentityColor.g,
                                   root.displayIdentityColor.b,
                                   0.10)
                    border.width: 1
                    border.color: Qt.rgba(root.displayIdentityColor.r,
                                          root.displayIdentityColor.g,
                                          root.displayIdentityColor.b,
                                          0.42)

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.sourceTrustTitle")
                        color: root.mangaText
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                    }
                }
            }

            Row {
                width: parent.width
                height: 56
                spacing: 10

                Rectangle {
                    width: (parent.width - 10) * 0.5
                    height: 56
                    radius: 10
                    color: AuroraTokens.mangaWash
                    border.width: 1
                    border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                          AuroraTokens.mangaInk.g,
                                          AuroraTokens.mangaInk.b,
                                          0.24)

                    Column {
                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 2

                        Text {
                            text: MusicSources.sourceCount
                            color: root.mangaText
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: AuroraI18n.text("music.sourceConnected")
                            color: root.mangaMutedText
                            font.pixelSize: 9
                        }
                    }
                }

                Rectangle {
                    width: (parent.width - 10) * 0.5
                    height: 56
                    radius: 10
                    color: Qt.rgba(root.displayIdentityColor.r,
                                   root.displayIdentityColor.g,
                                   root.displayIdentityColor.b,
                                   0.08)
                    border.width: 1
                    border.color: Qt.rgba(root.displayIdentityColor.r,
                                          root.displayIdentityColor.g,
                                          root.displayIdentityColor.b,
                                          0.30)

                    Column {
                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 2

                        Text {
                            text: MusicSources.onlineTrackCount
                            color: root.mangaText
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: AuroraI18n.text("music.onlineTracksReady")
                            color: root.mangaMutedText
                            font.pixelSize: 9
                        }
                    }
                }
            }

            Text {
                width: parent.width
                text: AuroraI18n.text("music.sourceHint")
                color: root.mangaText
                font.pixelSize: 11
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Rectangle {
                width: parent.width
                height: 86
                radius: 10
                color: AuroraTokens.mangaPaper
                border.width: 2
                border.color: sourceInput.activeFocus
                              ? root.displayIdentityColor
                              : Qt.rgba(AuroraTokens.mangaInk.r,
                                        AuroraTokens.mangaInk.g,
                                        AuroraTokens.mangaInk.b,
                                        0.72)

                TextEdit {
                    id: sourceInput

                    anchors.fill: parent
                    anchors.margins: 11
                    color: root.mangaText
                    selectionColor: Qt.rgba(root.displayIdentityColor.r,
                                            root.displayIdentityColor.g,
                                            root.displayIdentityColor.b, 0.42)
                    selectedTextColor: AuroraTokens.mangaInk
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    clip: true

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        text: AuroraI18n.text("music.sourcePlaceholder")
                        color: root.mangaMutedText
                        font.pixelSize: 11
                        visible: sourceInput.text.length === 0
                                 && !sourceInput.activeFocus
                        elide: Text.ElideRight
                    }
                }
            }

            Row {
                width: parent.width
                height: 40
                spacing: 10

                Rectangle {
                    width: parent.width - 138
                    height: 40
                    radius: 9
                    color: sourceInput.text.trim().length > 0
                           ? Qt.rgba(root.displayIdentityColor.r,
                                     root.displayIdentityColor.g,
                                     root.displayIdentityColor.b,
                                     0.22)
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: root.sourceInputLooksLikeScript(sourceInput.text)
                              ? AuroraI18n.text("music.importSource")
                              : root.sourceInputLooksLikeResolver(sourceInput.text)
                                ? AuroraI18n.text("music.resolveSource")
                                : AuroraI18n.text("music.sourcePrimary")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: sourceInput.text.trim().length > 0
                                 && !MusicSources.busy
                                 && !MusicSources.resolving
                        onTapped: root.playSourceInput(false)
                    }
                }

                Rectangle {
                    width: 128
                    height: 40
                    radius: 9
                    color: sourceInput.text.trim().length > 0
                           && !root.sourceInputLooksLikeScript(sourceInput.text)
                           ? AuroraTokens.mangaPanel
                           : AuroraTokens.mangaWash
                    border.width: 2
                    border.color: AuroraTokens.mangaInk

                    Text {
                        anchors.centerIn: parent
                        text: AuroraI18n.text("music.sourceQueue")
                        color: root.mangaText
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    TapHandler {
                        enabled: sourceInput.text.trim().length > 0
                                 && !root.sourceInputLooksLikeScript(sourceInput.text)
                                 && !MusicSources.resolving
                        onTapped: root.playSourceInput(true)
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 58
                radius: 10
                color: Qt.rgba(root.displayIdentityColor.r,
                               root.displayIdentityColor.g,
                               root.displayIdentityColor.b,
                               0.07)
                border.width: 1
                border.color: Qt.rgba(root.displayIdentityColor.r,
                                      root.displayIdentityColor.g,
                                      root.displayIdentityColor.b,
                                      0.30)

                Row {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 8
                        height: 8
                        radius: 4
                        color: MusicSources.errorString.length > 0
                               ? AuroraTokens.warmAccent
                               : root.displayIdentityColor
                    }

                    Column {
                        width: parent.width - 18
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            width: parent.width
                            text: root.sourcePanelStatus
                            color: MusicSources.errorString.length > 0
                                   ? AuroraTokens.warmAccent
                                   : root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: AuroraI18n.text("music.sourceTrustBody")
                            color: root.mangaMutedText
                            font.pixelSize: 9
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        width: 132
        height: 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035
        radius: 8
        color: root.mangaButtonFill
        border.width: 2
        border.color: root.mangaButtonBorder

        Text {
            anchors.centerIn: parent
            text: AudioRuntime.hasTrack
                  ? AuroraI18n.text("music.changeMusic")
                  : AuroraI18n.text("music.chooseMusic")
            color: root.mangaText
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        TapHandler { onTapped: audioDialog.open() }

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
    }

    Item {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 28
        anchors.topMargin: 82
        width: 246
        height: libraryMenuExpanded
                ? ((LocalLibrary.firstRun || LocalLibrary.libraryRootCount > 0) ? 184 : 150)
                : 42
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }

        Rectangle {
            id: libraryMenuButton
            width: 150
            height: 42
            radius: 8
            color: root.libraryMenuExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.20)
                   : root.mangaButtonFill
            border.width: 2
            border.color: root.libraryMenuExpanded
                          ? root.displayIdentityColor
                          : root.mangaButtonBorder

            Text {
                anchors.centerIn: parent
                text: LocalLibrary.scanning
                      ? AuroraI18n.text("music.scanning") + " · " + LocalLibrary.scannedFileCount
                      : AuroraI18n.text("music.library") + " · " + LocalLibrary.trackCount
                color: root.mangaText
                font.pixelSize: 13
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            TapHandler {
                onTapped: root.libraryMenuExpanded = !root.libraryMenuExpanded
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: libraryMenuButton.bottom
            anchors.topMargin: 8
            width: parent.width
            height: LocalLibrary.firstRun || LocalLibrary.libraryRootCount > 0 ? 134 : 100
            visible: root.libraryMenuExpanded
            opacity: root.libraryMenuExpanded ? 1 : 0
            radius: 8
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: AuroraTokens.mangaInk

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 9

                Text {
                    width: parent.width
                    text: LocalLibrary.lastScanStatus.length > 0
                          ? LocalLibrary.lastScanStatus
                            + (LocalLibrary.scanning
                               ? " · " + LocalLibrary.scannedFileCount
                                 + " " + AuroraI18n.text("music.files")
                               : "")
                          : (LocalLibrary.firstRun
                             ? root.firstRunLibraryMessage
                             : LocalLibrary.trackCount
                               + " " + AuroraI18n.text("music.ready"))
                    color: LocalLibrary.errorString.length > 0
                           ? AuroraTokens.warmAccent
                           : root.mangaMutedText
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Row {
                    spacing: 8

                    Rectangle {
                        width: 104
                        height: 30
                        radius: 15
                        color: LocalLibrary.scanning
                               ? AuroraTokens.mangaWash
                               : AuroraTokens.mangaPaper
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.scanning
                                  ? AuroraI18n.text("music.cancelScan")
                                  : (LocalLibrary.firstRun
                                     && LocalLibrary.defaultMusicDirectoryAvailable
                                     ? AuroraI18n.text("music.scanMusic")
                                     : AuroraI18n.text("music.addFolder"))
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            onTapped: {
                                if (LocalLibrary.scanning)
                                    LocalLibrary.cancelScan()
                                else if (LocalLibrary.firstRun
                                         && LocalLibrary.defaultMusicDirectoryAvailable)
                                    LocalLibrary.scanDefaultMusicDirectory()
                                else
                                    libraryDialog.open()
                            }
                        }
                    }

                    Rectangle {
                        width: 104
                        height: 30
                        radius: 15
                        color: LocalLibrary.sourceCount > 0
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.18)
                               : AuroraTokens.mangaWash
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: LocalLibrary.sourceCount > 0
                                  ? AuroraI18n.text("music.playLibrary")
                                  : AuroraI18n.text("music.chooseFolder")
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            enabled: LocalLibrary.sourceCount > 0
                                     || !LocalLibrary.scanning
                            onTapped: {
                                if (LocalLibrary.sourceCount > 0)
                                    AudioRuntime.setQueue(LocalLibrary.playableUrls())
                                else
                                    libraryDialog.open()
                            }
                        }
                    }
                }

                Row {
                    width: parent.width
                    height: 30
                    spacing: 8
                    visible: LocalLibrary.libraryRootCount > 0

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 92
                        text: LocalLibrary.libraryRootsSummary
                        color: LocalLibrary.unavailableRootCount > 0
                               ? AuroraTokens.warmAccent
                               : root.mangaMutedText
                        font.pixelSize: 10
                        elide: Text.ElideMiddle
                    }

                    Rectangle {
                        width: 84
                        height: 30
                        radius: 15
                        color: LocalLibrary.scanning
                               ? AuroraTokens.mangaWash
                               : Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.16)
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: AuroraI18n.text("music.rescanAll")
                            color: root.mangaText
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            enabled: !LocalLibrary.scanning
                            onTapped: LocalLibrary.rescanLibraryRoots()
                        }
                    }
                }
            }
        }
    }

    Item {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 28
        anchors.bottomMargin: 28
        width: Math.min(336, root.width * 0.31)
        height: root.tracksPanelExpanded ? Math.min(438, root.height - 146) : 42
        visible: root.width >= 920
        enabled: root.controlsVisible
        opacity: root.controlOpacity
        scale: 0.965 + root.controlOpacity * 0.035

        Behavior on opacity { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 320; easing.type: Easing.OutCubic } }

        Rectangle {
            id: tracksButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 132
            height: 42
            radius: 8
            color: root.tracksPanelExpanded
                   ? Qt.rgba(root.displayIdentityColor.r,
                             root.displayIdentityColor.g,
                             root.displayIdentityColor.b, 0.20)
                   : root.mangaButtonFill
            border.width: 2
            border.color: root.tracksPanelExpanded
                          ? root.displayIdentityColor
                          : root.mangaButtonBorder

            Text {
                anchors.centerIn: parent
                text: (root.sourceTracksMode
                       ? AuroraI18n.text("music.sourceTracks")
                       : AuroraI18n.text("music.tracks"))
                      + (root.visiblePanelTrackCount > 0
                         ? " · " + root.visiblePanelTrackCount
                         : "")
                color: root.mangaText
                font.pixelSize: 13
                font.weight: Font.DemiBold
            }

            TapHandler {
                onTapped: root.tracksPanelExpanded = !root.tracksPanelExpanded
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.bottom: tracksButton.top
            anchors.bottomMargin: 10
            width: parent.width
            height: parent.height - tracksButton.height - 10
            visible: root.tracksPanelExpanded
            radius: 8
            color: AuroraTokens.mangaPanel
            border.width: 2
            border.color: AuroraTokens.mangaInk

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                Row {
                    width: parent.width
                    height: 28
                    spacing: 8

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 154
                        text: root.sourceTracksMode
                              ? AuroraI18n.text("music.sourceTracks")
                              : AuroraI18n.text("music.tracks")
                        color: root.mangaText
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        width: 60
                        height: 28
                        radius: 14
                        color: MusicSources.onlineTrackCount > 0
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.18)
                               : AuroraTokens.mangaWash
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: root.sourceTracksMode
                                  ? AuroraI18n.text("music.localTracks")
                                  : AuroraI18n.text("music.sourceShort")
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            onTapped: {
                                root.sourceTracksMode = !root.sourceTracksMode
                                if (root.sourceTracksMode)
                                    MusicSources.loadOnlineTracks()
                            }
                        }
                    }

                    Rectangle {
                        width: 78
                        height: 28
                        radius: 14
                        color: root.visiblePanelTrackCount > 0 && !MusicSources.resolving
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.18)
                               : AuroraTokens.mangaWash
                        border.width: 2
                        border.color: AuroraTokens.mangaInk

                        Text {
                            anchors.centerIn: parent
                            text: AuroraI18n.text("music.playList")
                            color: root.mangaText
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            enabled: root.visiblePanelTrackCount > 0 && !MusicSources.resolving
                            onTapped: {
                                if (root.sourceTracksMode)
                                    root.requestOnlineTrackTransition(0)
                                else
                                    AudioRuntime.setQueue(LocalLibrary.visiblePlayableUrls())
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: root.sourceTracksMode ? 0 : 34
                    visible: !root.sourceTracksMode
                    radius: 8
                    color: AuroraTokens.mangaPaper
                    border.width: 2
                    border.color: searchInput.activeFocus
                                  ? root.displayIdentityColor
                                  : AuroraTokens.mangaInk

                    TextInput {
                        id: searchInput
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        text: LocalLibrary.searchText
                        color: root.mangaText
                        selectionColor: Qt.rgba(root.displayIdentityColor.r,
                                                root.displayIdentityColor.g,
                                                root.displayIdentityColor.b, 0.42)
                        selectedTextColor: AuroraTokens.mangaInk
                        font.pixelSize: 12
                        clip: true
                        onTextEdited: LocalLibrary.searchText = text

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: AuroraI18n.text("music.searchLibrary")
                            color: root.mangaMutedText
                            font.pixelSize: 12
                            visible: searchInput.text.length === 0 && !searchInput.activeFocus
                        }
                    }
                }

                ListView {
                    id: libraryList
                    width: parent.width
                    height: root.sourceTracksMode ? 0 : parent.height - 90
                    visible: !root.sourceTracksMode
                    model: LocalLibrary.tracks
                    clip: true
                    spacing: 6

                    delegate: Rectangle {
                        required property int index
                        required property string title
                        required property string artist
                        required property string album
                        required property string artworkUrl
                        required property string fileName
                        required property string filePath
                        required property url url

                        width: libraryList.width
                        height: 58
                        radius: 8
                        color: AudioRuntime.filePath === filePath
                               ? Qt.rgba(root.displayIdentityColor.r,
                                         root.displayIdentityColor.g,
                                         root.displayIdentityColor.b, 0.16)
                               : Qt.rgba(AuroraTokens.mangaInk.r,
                                         AuroraTokens.mangaInk.g,
                                         AuroraTokens.mangaInk.b,
                                         0.045)
                        border.width: 2
                        border.color: AudioRuntime.filePath === filePath
                                      ? root.displayIdentityColor
                                      : Qt.rgba(AuroraTokens.mangaInk.r,
                                                AuroraTokens.mangaInk.g,
                                                AuroraTokens.mangaInk.b,
                                                0.20)

                        Column {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 4

                            Text {
                                width: parent.width
                                text: title.length > 0 ? title : fileName
                                color: root.mangaText
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: artist.length > 0
                                      ? artist + (album.length > 0 ? " · " + album : "")
                                      : fileName
                                color: root.mangaMutedText
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }

                        TapHandler {
                            onTapped: {
                                root.requestLocalTrackTransition(index, {
                                    title: title.length > 0 ? title : fileName,
                                    artist: artist,
                                    album: album,
                                    url: url.toString(),
                                    sourceId: filePath
                                })
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: libraryList.count === 0
                        text: LocalLibrary.searchText.length > 0
                              ? AuroraI18n.text("music.noMatches")
                              : (LocalLibrary.firstRun
                                 ? AuroraI18n.text("music.scanToBegin")
                                 : AuroraI18n.text("music.noTracks"))
                            color: root.mangaMutedText
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }

                ListView {
                    id: onlineList
                    width: parent.width
                    height: root.sourceTracksMode ? parent.height - 46 : 0
                    visible: root.sourceTracksMode
                    model: MusicSources.onlineTracks
                    clip: true
                    spacing: 6

                    delegate: Rectangle {
                        required property int index
                        required property string source
                        required property string songId
                        required property string title
                        required property string artist
                        required property string album

                        width: onlineList.width
                        height: 68
                        radius: 10
                        color: Qt.rgba(root.displayIdentityColor.r,
                                       root.displayIdentityColor.g,
                                       root.displayIdentityColor.b,
                                       0.055)
                        border.width: 1
                        border.color: Qt.rgba(AuroraTokens.mangaInk.r,
                                              AuroraTokens.mangaInk.g,
                                              AuroraTokens.mangaInk.b,
                                              0.18)

                        Rectangle {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: 48
                            height: 28
                            radius: 14
                            color: Qt.rgba(root.displayIdentityColor.r,
                                           root.displayIdentityColor.g,
                                           root.displayIdentityColor.b,
                                           0.14)
                            border.width: 1
                            border.color: Qt.rgba(root.displayIdentityColor.r,
                                                  root.displayIdentityColor.g,
                                                  root.displayIdentityColor.b,
                                                  0.34)

                            Text {
                                anchors.centerIn: parent
                                text: source.toUpperCase()
                                color: root.mangaText
                                font.pixelSize: 9
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }

                        Column {
                            anchors.left: parent.left
                            anchors.right: playGlyph.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 70
                            anchors.rightMargin: 10
                            spacing: 4

                            Text {
                                width: parent.width
                                text: title
                                color: root.mangaText
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: artist.length > 0
                                      ? artist + (album.length > 0 ? " · " + album : "")
                                      : songId
                                color: root.mangaMutedText
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            id: playGlyph
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "↗"
                            color: root.displayIdentityColor
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                        }

                        TapHandler {
                            enabled: !MusicSources.resolving
                            onTapped: root.playOnlineTrack(index)
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: onlineList.count === 0
                        text: MusicSources.catalogBusy
                              ? AuroraI18n.text("music.sourceCatalogLoading")
                              : AuroraI18n.text("music.noSourceTracks")
                        color: root.mangaMutedText
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    AudioDiagnostics {
        visible: false
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 28
        anchors.topMargin: 82
        diagnosticsVisible: root.diagnosticsVisible
        reactiveAvailable: AudioRuntime.audioReactiveAvailable
        playbackActive: AudioRuntime.playing
        audioLevel: AudioRuntime.audioLevel
        bassEnergy: AudioRuntime.bassEnergy
        midEnergy: AudioRuntime.midEnergy
        highEnergy: AudioRuntime.highEnergy
        transientEnergy: AudioRuntime.transientEnergy
    }

    Item {
        id: playerContent

        anchors.fill: parent
        anchors.leftMargin: root.compactViewport
                            ? (root.layoutControlsVisible ? 44 : 30)
                            : (root.layoutControlsVisible ? 72 : 48)
        anchors.rightMargin: root.compactViewport
                             ? (root.layoutControlsVisible ? 44 : 30)
                             : (root.layoutControlsVisible ? 72 : 48)
        anchors.topMargin: root.compactViewport
                           ? (root.layoutControlsVisible ? 54 : 34)
                           : (root.layoutControlsVisible ? 62 : 48)
        anchors.bottomMargin: root.compactViewport
                              ? (root.layoutControlsVisible ? 50 : 34)
                              : (root.layoutControlsVisible ? 58 : 42)
        opacity: networkTransition.running
                 && networkTransition.transitionState !== "PlaybackRevealing"
                 ? networkTransition.panelOpacity
                 : 1.0
        scale: networkTransition.running ? networkTransition.panelScale : 1.0
        enabled: !networkTransition.running
                 || networkTransition.transitionState === "PlaybackRevealing"
                 || networkTransition.transitionState === "Idle"
        z: 4

        readonly property bool wide: width >= 760
        readonly property real illustrationX:
            wide ? width * (root.layoutControlsVisible ? 0.27 : 0.25) : 0
        readonly property real illustrationY:
            wide ? height * 0.08 : height * 0.03
        readonly property real illustrationWidth:
            wide ? width * (root.layoutControlsVisible ? 0.72 : 0.75) : width
        readonly property real illustrationHeight:
            wide ? height * (root.layoutControlsVisible ? 0.76 : 0.79) : height * 0.58
        readonly property real identityWidth:
            wide ? Math.min(340, width * 0.29) : Math.min(560, width * 0.82)

        Behavior on opacity {
            enabled: networkTransition.transitionState !== "PlaybackRevealing"
            OpacityAnimator {
                duration: root.reducedMotion ? AuroraTokens.motionSoft : 440
                easing.type: Easing.OutQuint
            }
        }

        Behavior on scale {
            ScaleAnimator {
                duration: root.reducedMotion ? AuroraTokens.motionSoft : 500
                easing.type: Easing.OutQuint
            }
        }

        MusicIllustrationSpace {
            id: illustrationSpace

            x: playerContent.illustrationX
            y: playerContent.illustrationY
            width: playerContent.illustrationWidth
            height: playerContent.illustrationHeight
            title: root.displayTitle
            artist: root.displayArtist
            trackIdentity: AudioRuntime.trackId.length > 0
                           ? AudioRuntime.trackId
                           : root.displayTitle + "|" + root.displayArtist
            artworkAvailable: AudioRuntime.artworkSource.toString().length > 0
                              && AudioRuntime.identityProvenance !== "Generated Identity"
            artworkIllustrationSource: ArtworkIllustration.illustrationSource
            artworkReady: ArtworkIllustration.ready
            artworkFocalX: ArtworkIllustration.focalX
            artworkFocalY: ArtworkIllustration.focalY
            artworkEdgeDensity: ArtworkIllustration.edgeDensity
            artworkContrast: ArtworkIllustration.contrast
            artworkParticles: ArtworkIllustration.particleSamples
            colorSignature: root.displayIdentityColor
            playing: AudioRuntime.playing
            reducedMotion: root.reducedMotion
            qualityMode: root.qualityMode
            audioReactiveAvailable: AudioRuntime.audioReactiveAvailable
                                    && !root.transitionCrystalSettling
            audioLevel: root.transitionCrystalSettling ? 0.0 : AudioRuntime.audioLevel
            bassEnergy: root.transitionCrystalSettling ? 0.0 : AudioRuntime.bassEnergy
            midEnergy: root.transitionCrystalSettling ? 0.0 : AudioRuntime.midEnergy
            highEnergy: root.transitionCrystalSettling ? 0.0 : AudioRuntime.highEnergy
            transientEnergy: root.transitionCrystalSettling ? 0.0 : AudioRuntime.transientEnergy
            opacity: root.identityVisible
                     ? root.transitionLayerAmount(0.16, 0.62)
                     : 0.0

            Behavior on x {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 560; easing.type: Easing.OutQuint }
            }
            Behavior on y {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 560; easing.type: Easing.OutQuint }
            }
            Behavior on width {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 560; easing.type: Easing.OutQuint }
            }
            Behavior on height {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 560; easing.type: Easing.OutQuint }
            }
            Behavior on opacity {
                NumberAnimation { duration: AuroraTokens.motionSoft; easing.type: Easing.OutCubic }
            }
        }


        Item {
            id: identityCopy

            x: playerContent.wide
               ? parent.width * 0.06
               : (parent.width - playerContent.identityWidth) / 2
            y: playerContent.wide
               ? parent.height * (root.layoutControlsVisible ? 0.20 : 0.23)
               : illustrationSpace.y + illustrationSpace.height + 18
            width: playerContent.identityWidth
            height: playerContent.wide ? Math.min(330, parent.height * 0.58) : 220
            opacity: root.transitionLayerAmount(0.36, 0.86)

            Rectangle {
                width: Math.min(146, parent.width * 0.44)
                height: 3
                radius: 2
                color: root.displayIdentityColor
                opacity: root.controlsVisible ? 0.64 : 0.84
            }

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 22
                spacing: 10

                Text {
                    width: parent.width
                    visible: root.controlsVisible && AudioRuntime.hasTrack
                    text: root.playbackTrustLabel.toUpperCase()
                    color: root.displayIdentityColor
                    opacity: 0.82
                    font.pixelSize: 10
                    font.letterSpacing: 1.2
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: root.displayTitle
                    color: root.mangaText
                    font.pixelSize: root.controlsVisible ? 36 : 48
                    font.weight: Font.DemiBold
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    visible: root.displayArtistLine.length > 0 || root.displayAlbum.length > 0
                    text: root.displayArtistLine
                          + (root.displayAlbum.length > 0
                             ? (root.displayArtistLine.length > 0 ? " · " : "")
                               + root.displayAlbum
                             : "")
                    color: root.mangaMutedText
                    opacity: root.controlsVisible ? 0.88 : 0.66
                    font.pixelSize: root.controlsVisible ? 15 : 16
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }

                Rectangle {
                    visible: AudioRuntime.errorString.length > 0
                    width: parent.width
                    height: 58
                    radius: 10
                    color: Qt.rgba(AuroraTokens.warmAccent.r,
                                   AuroraTokens.warmAccent.g,
                                   AuroraTokens.warmAccent.b,
                                   0.09)
                    border.width: 1
                    border.color: AuroraTokens.warmAccent

                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 116
                            text: AuroraI18n.text("music.playbackUnavailable")
                            color: AuroraTokens.warmAccent
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 106
                            height: 34
                            radius: 8
                            color: AuroraTokens.mangaPanel
                            border.width: 1
                            border.color: AuroraTokens.mangaInk

                            Text {
                                anchors.centerIn: parent
                                text: AuroraI18n.text("music.reconnectSource")
                                color: AuroraTokens.mangaInk
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                            }

                            TapHandler {
                                onTapped: {
                                    root.sourcePanelExpanded = true
                                    root.revealControls()
                                }
                            }
                        }
                    }
                }
            }


            // AUR-CONTROLS-REFERENCE-PACK-04:TRANSPORT-BEGIN
            Item {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: playerContent.wide ? 0 : 0
                width: 440
                height: 112
                opacity: root.transitionLayerAmount(0.66, 1.0)
                z: 140

                MangekyoEyeButton {
                    id: previousMangekyoButton

                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed:
                        root.controlsVisible
                        && !networkTransition.running
                    reducedMotion: root.reducedMotion
                    direction: -1
                    onActivated: root.beginTrackTransition(-1)
                }

                LamborghiniPlayToggle {
                    id: lamborghiniTransportButton

                    anchors.centerIn: parent
                    playing: AudioRuntime.playing
                    hasTrack: AudioRuntime.hasTrack
                    reducedMotion: root.reducedMotion

                    onActivated: {
                        if (AudioRuntime.hasTrack)
                            AudioRuntime.togglePlayback()
                        else
                            audioDialog.open()
                    }
                }

                MangekyoEyeButton {
                    id: nextMangekyoButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed:
                        root.controlsVisible
                        && !networkTransition.running
                    reducedMotion: root.reducedMotion
                    direction: 1
                    onActivated: root.beginTrackTransition(1)
                }
            }
            // AUR-CONTROLS-REFERENCE-PACK-04:TRANSPORT-END

            Behavior on x {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 520; easing.type: Easing.OutQuint }
            }
            Behavior on y {
                enabled: !root.reducedMotion
                NumberAnimation { duration: 520; easing.type: Easing.OutQuint }
            }
        }

        Item {
            id: lyricBand

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: timeTrace.top
            anchors.bottomMargin: root.controlsVisible ? 12 : 8
            height: Lyrics.loading
                    ? 34
                    : (Lyrics.hasLyrics && Lyrics.currentLine.length > 0
                       ? (Lyrics.nextLine.length > 0 ? 54 : 34)
                       : 0)
            visible: height > 0
            opacity: (root.controlsVisible ? 0.88 : 0.68)
                     * root.transitionLayerAmount(0.54, 0.94)

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: playerContent.wide ? parent.width * 0.05 : 18
                anchors.rightMargin: playerContent.wide ? parent.width * 0.18 : 18
                spacing: 5

                Text {
                    width: parent.width
                    text: Lyrics.loading ? "正在读取歌词…" : Lyrics.currentLine
                    color: root.mangaText
                    font.pixelSize: root.controlsVisible ? 16 : 15
                    font.weight: Font.Medium
                    wrapMode: Text.Wrap
                    maximumLineCount: 1
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    visible: !Lyrics.loading && text.length > 0
                    text: Lyrics.nextLine
                    color: root.mangaMutedText
                    opacity: 0.50
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    maximumLineCount: 1
                    elide: Text.ElideRight
                }
            }

            Behavior on height {
                NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
            }
            Behavior on opacity {
                NumberAnimation { duration: 260; easing.type: Easing.OutCubic }
            }
        }

        Item {
            id: timeTrace

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: root.controlsVisible ? 42 : 18
            opacity: root.transitionLayerAmount(0.68, 1.0)

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: root.formatTime(AudioRuntime.position)
                color: root.mangaMutedText
                font.pixelSize: 12
                opacity: root.controlOpacity
            }

            Rectangle {
                id: progressTrack
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: root.controlsVisible ? 52 : parent.width * 0.22
                anchors.rightMargin: root.controlsVisible ? 52 : parent.width * 0.22
                anchors.verticalCenter: parent.verticalCenter
                height: root.controlsVisible ? (root.presentationMode ? 4 : 5) : 2
                radius: height / 2
                opacity: root.controlsVisible ? 1.0 : AudioRuntime.hasTrack ? 0.42 : 0.0
                color: Qt.rgba(AuroraTokens.mangaInk.r,
                               AuroraTokens.mangaInk.g,
                               AuroraTokens.mangaInk.b,
                               root.controlsVisible ? 0.16 : 0.060)

                Rectangle {
                    width: parent.width * root.progressRatio
                    height: parent.height
                    radius: parent.radius
                    color: root.displayIdentityColor
                }

                TapHandler {
                    id: progressTap
                    enabled: root.controlsVisible && AudioRuntime.duration > 0
                    onTapped: {
                        AudioRuntime.position = AudioRuntime.duration
                                                * Math.max(0, Math.min(1,
                                                    progressTap.point.position.x / progressTrack.width))
                    }
                }
            }

            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: root.formatTime(AudioRuntime.duration)
                color: root.mangaMutedText
                font.pixelSize: 12
                opacity: root.controlOpacity
            }
        }

        Row {
            anchors.left: parent.left
            anchors.bottom: lyricBand.visible ? lyricBand.top : timeTrace.top
            anchors.bottomMargin: 8
            spacing: 10
            enabled: root.controlsVisible
            opacity: root.controlOpacity * root.transitionLayerAmount(0.72, 1.0)
            scale: 0.97 + root.controlOpacity * 0.03

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: AuroraI18n.text("music.volume")
                color: root.mangaMutedText
                font.pixelSize: 12
            }

            Rectangle {
                id: volumeTrack
                width: 132
                height: 5
                radius: 3
                color: Qt.rgba(AuroraTokens.mangaInk.r,
                               AuroraTokens.mangaInk.g,
                               AuroraTokens.mangaInk.b,
                               0.16)

                Rectangle {
                    width: parent.width * AudioRuntime.volume
                    height: parent.height
                    radius: parent.radius
                    color: root.displayIdentityColor
                }

                TapHandler {
                    id: volumeTap
                    onTapped: {
                        AudioRuntime.volume = Math.max(0, Math.min(1,
                                                volumeTap.point.position.x / volumeTrack.width))
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Math.round(AudioRuntime.volume * 100) + "%"
                color: root.mangaMutedText
                font.pixelSize: 12
            }

            Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
            Behavior on scale { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
        }

    }

    MusicNetworkTransitionOverlay {
        id: networkTransition
        x: playerContent.x
        y: playerContent.y
        width: playerContent.width
        height: playerContent.height
        accessibilityMode: root.accessibilityMode
        qualityMode: root.qualityMode
        colorSignature: root.displayIdentityColor

        onPlaybackRequested: function(targetTrack) {
            root.executeNetworkTransitionAction(root.activeNetworkTransitionAction)
        }
        onPlaybackTimedOut: {
            root.resetNetworkTransitionRuntime()
            root.transitionLanding = false
            root.transitionCrystalSettling = false
        }
        onTransitionFinished: {
            root.transitionLanding = false
            crystalSettleTimer.restart()
            root.resetNetworkTransitionRuntime()
            root.transitionLayoutControlsVisible = root.controlsVisible
            controlHideTimer.restart()

            if (root.pendingNetworkTransitionAction !== null) {
                const nextAction = root.pendingNetworkTransitionAction
                root.pendingNetworkTransitionAction = null
                Qt.callLater(function() {
                    root.requestNetworkTransition(nextAction)
                })
            }
        }
    }

}
