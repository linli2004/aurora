import QtQuick
import Qt5Compat.GraphicalEffects
import Aurora.Runtime 1.0

Item {
    id: root

    property string title: ""
    property string artist: ""
    property string trackIdentity: ""
    property bool artworkAvailable: false
    property url artworkIllustrationSource: ""
    property bool artworkReady: false
    property real artworkFocalX: 0.5
    property real artworkFocalY: 0.5
    property real artworkEdgeDensity: 0.0
    property real artworkContrast: 0.0
    property var artworkParticles: []
    property color colorSignature: AuroraTokens.coolAccent
    property bool playing: false
    property bool reducedMotion: false
    property int qualityMode: AuroraTypes.Balanced
    property bool audioReactiveAvailable: false
    property real audioLevel: 0.0
    property real bassEnergy: 0.0
    property real midEnergy: 0.0
    property real highEnergy: 0.0
    property real transientEnergy: 0.0
    property real phase: 0.0
    property real explosion: 0.0
    property real pitchMemory: 0.0

    readonly property int shardColumns:
        qualityMode === AuroraTypes.Immersive ? 18
        : qualityMode === AuroraTypes.Eco ? 9 : 15
    readonly property int shardRows: shardColumns
    readonly property real fragmentProgress:
        reducedMotion ? 0.0
        : Math.max(explosion, pitchMemory * 0.82)
    readonly property real sourcePixelWidth:
        Math.max(1.0, artworkImage.sourceSize.width)
    readonly property real sourcePixelHeight:
        Math.max(1.0, artworkImage.sourceSize.height)
    readonly property real artworkDisplayWidth:
        Math.max(1.0, artworkImage.paintedWidth)
    readonly property real artworkDisplayHeight:
        Math.max(1.0, artworkImage.paintedHeight)
    readonly property real artworkDisplayX:
        artworkImage.x
        + (artworkImage.width - artworkDisplayWidth) * 0.5
    readonly property real artworkDisplayY:
        artworkImage.y
        + (artworkImage.height - artworkDisplayHeight) * 0.5

    readonly property bool motionEnabled:
        visible && !reducedMotion && qualityMode !== AuroraTypes.Eco
    readonly property real level:
        audioReactiveAvailable ? Math.max(0.0, Math.min(1.0, audioLevel)) : 0.0
    readonly property real bass:
        audioReactiveAvailable ? Math.max(0.0, Math.min(1.0, bassEnergy)) : 0.0
    readonly property real mid:
        audioReactiveAvailable ? Math.max(0.0, Math.min(1.0, midEnergy)) : 0.0
    readonly property real high:
        audioReactiveAvailable ? Math.max(0.0, Math.min(1.0, highEnergy)) : 0.0
    readonly property real transientLevel:
        audioReactiveAvailable ? Math.max(0.0, Math.min(1.0, transientEnergy)) : 0.0
    readonly property real spectralTotal:
        Math.max(0.001, bass + mid + high)
    readonly property real pitchDrive:
        Math.max(0.0, Math.min(1.0,
            high * 0.72
            + (high / spectralTotal) * 0.48
            + transientLevel * 0.22
            - 0.16))
    readonly property bool particleMapReady:
        artworkParticles !== null
        && artworkParticles !== undefined
        && artworkParticles.length > 0
    readonly property real explosionTarget:
        playing && audioReactiveAvailable && artworkReady
        ? Math.pow(
            Math.max(0.0, Math.min(1.0,
                high * 1.04
                + (high / spectralTotal) * 0.74
                + transientLevel * 0.44
                + mid * 0.20
                + bass * 0.10
                - 0.025)),
            0.86)
        : 0.0
    readonly property int seed:
        hashString(trackIdentity.length > 0
                   ? trackIdentity
                   : title + "|" + artist)
    readonly property int templateIndex: seed % 3

    function hashString(value) {
        let hash = 0
        for (let index = 0; index < value.length; ++index) {
            hash = ((hash << 5) - hash) + value.charCodeAt(index)
            hash |= 0
        }
        return Math.abs(hash)
    }

    function noise(index, salt) {
        const value = Math.sin((index + 1) * 12.9898
                               + (root.seed % 65521) * 0.0017
                               + salt * 78.233) * 43758.5453
        return value - Math.floor(value)
    }

    function ink(alpha) {
        return "rgba(35,31,28,"
                + Math.max(0.0, Math.min(1.0, alpha)) + ")"
    }

    function paper(alpha) {
        return "rgba(250,246,237,"
                + Math.max(0.0, Math.min(1.0, alpha)) + ")"
    }

    function shardColor(tone, alpha) {
        const clampedTone = Math.max(0.0, Math.min(1.0, tone))
        const value = Math.round(
            24 + clampedTone * 208)
        return "rgba("
                + value + ","
                + value + ","
                + Math.max(16, value - 5) + ","
                + Math.max(0.0, Math.min(1.0, alpha))
                + ")"
    }

    function drawExplosionRings(context, width, height) {
        // Intentionally disabled in Pack 06.
    }

    function drawArtworkParticles(context, width, height) {
        // Pack 07 renders the actual image as cropped fragments in QML.
    }

    function drawArtworkMotion(context, width, height) {
        // Artwork-ready scenes are rendered by the shattered image field.
    }

    function drawFallback(context, width, height) {
        const centerX = width * (0.48 + (root.noise(3, 47) - 0.5) * 0.12)
        const centerY = height * (0.48 + (root.noise(5, 53) - 0.5) * 0.10)
        const lineCount = root.qualityMode === AuroraTypes.Immersive ? 28
                          : root.qualityMode === AuroraTypes.Eco ? 10 : 19
        const steps = root.qualityMode === AuroraTypes.Eco ? 54 : 86

        if (root.templateIndex === 0) {
            for (let line = 0; line < lineCount; ++line) {
                const spread = line / Math.max(1, lineCount - 1) - 0.5
                context.beginPath()
                for (let step = 0; step <= steps; ++step) {
                    const progress = step / steps
                    const angle = progress * Math.PI * 1.8
                            + spread * 1.5
                            + root.phase * 0.07
                    const radius = Math.min(width, height)
                            * (0.08 + progress * 0.42)
                    const x = centerX + Math.cos(angle) * radius * 1.28
                    const y = centerY + Math.sin(angle) * radius * 0.72
                            + spread * height * 0.20 * (1.0 - progress)
                    if (step === 0)
                        context.moveTo(x, y)
                    else
                        context.lineTo(x, y)
                }
                context.lineWidth = line % 6 === 0 ? 0.95 : 0.44
                context.strokeStyle = root.ink(
                    0.055 + (line % 6 === 0 ? 0.055 : 0.0))
                context.stroke()
            }
        } else if (root.templateIndex === 1) {
            const count = root.qualityMode === AuroraTypes.Immersive ? 210
                          : root.qualityMode === AuroraTypes.Eco ? 58 : 125
            for (let index = 0; index < count; ++index) {
                const angle = root.noise(index, 61) * Math.PI * 2.0
                const radius = Math.min(width, height)
                        * (0.08 + Math.pow(root.noise(index, 67), 0.62) * 0.43)
                const drift = root.phase * (0.01 + root.noise(index, 71) * 0.01)
                const x = centerX + Math.cos(angle + drift) * radius * 1.28
                const y = centerY + Math.sin(angle + drift) * radius * 0.76
                const size = 0.6 + root.noise(index, 73) * 2.0
                context.fillStyle = root.ink(
                    0.045 + root.noise(index, 79) * 0.17)
                context.fillRect(x, y, size, size)
            }
        } else {
            for (let line = 0; line < lineCount + 8; ++line) {
                context.beginPath()
                const progressY = line / Math.max(1, lineCount + 7)
                for (let step = 0; step <= steps; ++step) {
                    const progress = step / steps
                    const x = width * (0.07 + progress * 0.89)
                    const dx = (x - centerX) / Math.max(1, width * 0.25)
                    const hollow = Math.exp(-dx * dx * 2.8)
                    const y = height * (0.15 + progressY * 0.70)
                            - hollow * height * 0.12
                            + Math.sin(progress * Math.PI * (3 + line % 5)
                                       + line * 0.25 + root.phase * 0.12)
                              * (2.0 + root.mid * 3.0)
                    if (step === 0)
                        context.moveTo(x, y)
                    else
                        context.lineTo(x, y)
                }
                context.lineWidth = line % 7 === 0 ? 0.86 : 0.40
                context.strokeStyle = root.ink(
                    0.047 + (line % 7 === 0 ? 0.055 : 0.0))
                context.stroke()
            }
        }

        const ringCount = root.qualityMode === AuroraTypes.Eco ? 4 : 7
        for (let ring = 0; ring < ringCount; ++ring) {
            const radius = Math.min(width, height) * (0.055 + ring * 0.010)
                    * (1.0 + root.bass * 0.024)
            context.beginPath()
            context.arc(centerX, centerY, radius, 0, Math.PI * 2.0)
            context.lineWidth = ring === 0 ? 1.1 : 0.44
            context.strokeStyle = root.ink(0.10 + (ringCount - ring) * 0.018)
            context.stroke()
        }
    }

    function paintScene(context) {
        const width = motionCanvas.width
        const height = motionCanvas.height
        if (width <= 1 || height <= 1)
            return

        context.clearRect(0, 0, width, height)
        context.lineCap = "round"
        context.lineJoin = "round"

        if (root.artworkReady)
            root.drawArtworkMotion(context, width, height)
        else
            root.drawFallback(context, width, height)
    }

    Timer {
        interval: root.qualityMode === AuroraTypes.Immersive ? 34
                  : root.qualityMode === AuroraTypes.Eco ? 80 : 50
        repeat: true
        running: root.visible && !root.reducedMotion
        onTriggered: {
            const attack = root.explosionTarget > root.explosion
                    ? 0.48
                    : 0.018
            root.explosion +=
                    (root.explosionTarget - root.explosion)
                    * attack

            const memoryTarget = Math.max(
                root.pitchDrive,
                root.explosionTarget * 0.98)
            const memoryFactor =
                memoryTarget > root.pitchMemory
                ? 0.36
                : 0.010
            root.pitchMemory +=
                    (memoryTarget - root.pitchMemory)
                    * memoryFactor

            if (root.motionEnabled) {
                root.phase = (root.phase
                              + (root.playing
                                 ? 0.022
                                   + root.explosion * 0.030
                                   + root.high * 0.011
                                 : 0.004))
                        % (Math.PI * 2.0)
            }

            motionCanvas.requestPaint()
        }
    }

    Rectangle {
        id: mirrorPane

        anchors.centerIn: parent
        width: artworkImage.paintedWidth + 4
        height: artworkImage.paintedHeight + 4
        visible: artworkImage.status === Image.Ready
        color: root.paper(0.96)
        border.width: 1
        border.color: root.ink(0.24)
        opacity: Math.max(
            0.0,
            0.98 - root.fragmentProgress * 1.18)
    }

    Image {
        id: artworkImage

        anchors.centerIn: parent
        width: parent.width * 0.94
        height: parent.height * 0.94
        source: root.artworkIllustrationSource
        visible: root.artworkReady
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        asynchronous: true
        cache: false
        opacity: status === Image.Ready
                 ? Math.max(
                     0.02,
                     0.99 - root.fragmentProgress * 1.12)
                 : 0.0
        scale: 1.0
               + root.bass * 0.002

        Behavior on opacity {
            NumberAnimation {
                duration: root.reducedMotion ? 0 : 420
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            enabled: !root.reducedMotion
            NumberAnimation {
                duration: 220
                easing.type: Easing.OutCubic
            }
        }

        onStatusChanged: {
            motionCanvas.requestPaint()
        }
    }

    Item {
        id: shatteredArtwork

        anchors.fill: parent
        visible: artworkImage.status === Image.Ready
                 && root.artworkReady
                 && root.fragmentProgress > 0.006
        z: artworkImage.z + 1

        Repeater {
            model: root.shardColumns * root.shardRows

            delegate: Item {
                id: shard

                required property int index

                readonly property int column:
                    index % root.shardColumns
                readonly property int row:
                    Math.floor(index / root.shardColumns)
                readonly property real normalizedX:
                    (column + 0.5) / root.shardColumns
                readonly property real normalizedY:
                    (row + 0.5) / root.shardRows
                readonly property real sourceX:
                    Math.floor(
                        column
                        * root.sourcePixelWidth
                        / root.shardColumns)
                readonly property real sourceY:
                    Math.floor(
                        row
                        * root.sourcePixelHeight
                        / root.shardRows)
                readonly property real sourceRight:
                    Math.ceil(
                        (column + 1)
                        * root.sourcePixelWidth
                        / root.shardColumns)
                readonly property real sourceBottom:
                    Math.ceil(
                        (row + 1)
                        * root.sourcePixelHeight
                        / root.shardRows)
                readonly property real sourceWidth:
                    Math.max(1.0, sourceRight - sourceX)
                readonly property real sourceHeight:
                    Math.max(1.0, sourceBottom - sourceY)
                readonly property real baseWidth:
                    root.artworkDisplayWidth / root.shardColumns
                readonly property real baseHeight:
                    root.artworkDisplayHeight / root.shardRows
                readonly property real baseX:
                    root.artworkDisplayX + column * baseWidth
                readonly property real baseY:
                    root.artworkDisplayY + row * baseHeight
                readonly property real vectorX:
                    normalizedX - root.artworkFocalX
                readonly property real vectorY:
                    normalizedY - root.artworkFocalY
                readonly property real vectorLength:
                    Math.max(
                        0.025,
                        Math.sqrt(
                            vectorX * vectorX
                            + vectorY * vectorY))
                readonly property real normalX:
                    vectorX / vectorLength
                readonly property real normalY:
                    vectorY / vectorLength
                readonly property real tangentX: -normalY
                readonly property real tangentY: normalX
                readonly property real randomA:
                    root.noise(index, 211)
                readonly property real randomB:
                    root.noise(index, 223)
                readonly property real randomC:
                    root.noise(index, 239)
                readonly property real cornerNoiseA:
                    root.noise(index, 251)
                readonly property real cornerNoiseB:
                    root.noise(index, 263)
                readonly property real cornerNoiseC:
                    root.noise(index, 271)
                readonly property real cornerNoiseD:
                    root.noise(index, 283)
                readonly property real cornerNoiseE:
                    root.noise(index, 293)
                readonly property real cornerNoiseF:
                    root.noise(index, 307)
                readonly property real cornerNoiseG:
                    root.noise(index, 317)
                readonly property real cornerNoiseH:
                    root.noise(index, 331)
                readonly property int shardVariant:
                    Math.floor(randomA * 5.0)
                readonly property real threshold:
                    0.02 + randomA * 0.20
                readonly property real activation:
                    Math.max(
                        0.0,
                        Math.min(
                            1.0,
                            (root.fragmentProgress - threshold)
                            / Math.max(0.10, 1.0 - threshold)))
                readonly property real blastDistance:
                    Math.min(root.width, root.height)
                    * Math.pow(activation, 1.20)
                    * (0.08 + randomB * 0.48)
                    * (0.78 + root.pitchMemory * 0.92)
                readonly property real transientKick:
                    root.transientLevel
                    * Math.min(root.width, root.height)
                    * (0.012 + randomC * 0.075)
                readonly property real hover:
                    Math.sin(
                        root.phase * (0.70 + randomA * 1.20)
                        + randomB * Math.PI * 8.0)
                    * Math.min(root.width, root.height)
                    * (0.003 + randomC * 0.020)
                    * (0.18 + activation * 0.82)
                readonly property real sideDrift:
                    Math.cos(
                        root.phase * (0.40 + randomC * 0.82)
                        + randomA * Math.PI * 6.0)
                    * Math.min(root.width, root.height)
                    * (0.002 + randomB * 0.030)
                    * activation

                readonly property real shardPad:
                    Math.max(1.5, Math.min(baseWidth, baseHeight) * 0.12)
                readonly property real usableWidth:
                    Math.max(6.0, width - shardPad * 2.0)
                readonly property real usableHeight:
                    Math.max(6.0, height - shardPad * 2.0)

                function polygonPoints() {
                    const px = shardPad
                    const py = shardPad
                    const uw = usableWidth
                    const uh = usableHeight
                    const left = px
                    const right = px + uw
                    const top = py
                    const bottom = py + uh
                    const cx = px + uw * (0.44 + (cornerNoiseG - 0.5) * 0.18)
                    const cy = py + uh * (0.48 + (cornerNoiseH - 0.5) * 0.18)
                    const topA = px + uw * (0.12 + cornerNoiseA * 0.18)
                    const topB = px + uw * (0.66 + cornerNoiseB * 0.20)
                    const rightA = py + uh * (0.12 + cornerNoiseC * 0.20)
                    const rightB = py + uh * (0.56 + cornerNoiseD * 0.18)
                    const bottomA = px + uw * (0.62 + cornerNoiseE * 0.20)
                    const bottomB = px + uw * (0.18 + cornerNoiseF * 0.20)
                    const leftA = py + uh * (0.62 + cornerNoiseG * 0.16)
                    const leftB = py + uh * (0.18 + cornerNoiseH * 0.18)

                    if (shardVariant === 0) {
                        return [
                            { x: left + uw * (0.02 + cornerNoiseA * 0.06), y: topA },
                            { x: topB, y: top + uh * (0.01 + cornerNoiseB * 0.05) },
                            { x: right - uw * (0.02 + cornerNoiseC * 0.05), y: rightA },
                            { x: cx + uw * (0.06 + cornerNoiseD * 0.10), y: cy },
                            { x: bottomA, y: bottom - uh * (0.02 + cornerNoiseE * 0.05) },
                            { x: left + uw * (0.04 + cornerNoiseF * 0.07), y: leftA }
                        ]
                    }

                    if (shardVariant === 1) {
                        return [
                            { x: topA, y: top },
                            { x: right - uw * (0.03 + cornerNoiseB * 0.05), y: rightA },
                            { x: right - uw * (0.18 + cornerNoiseC * 0.10), y: rightB },
                            { x: bottomA, y: bottom },
                            { x: bottomB, y: bottom - uh * (0.04 + cornerNoiseD * 0.06) },
                            { x: left + uw * (0.03 + cornerNoiseE * 0.05), y: leftB },
                            { x: left + uw * (0.01 + cornerNoiseF * 0.04), y: top + uh * (0.26 + cornerNoiseG * 0.12) }
                        ]
                    }

                    if (shardVariant === 2) {
                        return [
                            { x: left + uw * (0.08 + cornerNoiseA * 0.08), y: top + uh * (0.02 + cornerNoiseB * 0.06) },
                            { x: cx, y: top },
                            { x: right - uw * (0.04 + cornerNoiseC * 0.06), y: top + uh * (0.18 + cornerNoiseD * 0.12) },
                            { x: right, y: cy },
                            { x: right - uw * (0.16 + cornerNoiseE * 0.10), y: bottom - uh * (0.03 + cornerNoiseF * 0.06) },
                            { x: cx + uw * 0.04, y: bottom },
                            { x: left + uw * (0.06 + cornerNoiseG * 0.08), y: bottom - uh * (0.12 + cornerNoiseH * 0.10) },
                            { x: left, y: cy - uh * 0.04 }
                        ]
                    }

                    if (shardVariant === 3) {
                        return [
                            { x: left + uw * (0.03 + cornerNoiseA * 0.04), y: top + uh * (0.12 + cornerNoiseB * 0.10) },
                            { x: topB, y: top },
                            { x: right - uw * (0.05 + cornerNoiseC * 0.04), y: rightA },
                            { x: cx + uw * 0.10, y: cy + uh * (0.02 + cornerNoiseD * 0.08) },
                            { x: bottomA, y: bottom - uh * (0.01 + cornerNoiseE * 0.04) },
                            { x: left + uw * (0.12 + cornerNoiseF * 0.08), y: bottom },
                            { x: left, y: leftB }
                        ]
                    }

                    return [
                        { x: left + uw * (0.05 + cornerNoiseA * 0.06), y: top + uh * (0.03 + cornerNoiseB * 0.08) },
                        { x: topA, y: top },
                        { x: topB, y: top + uh * (0.01 + cornerNoiseC * 0.04) },
                        { x: right - uw * (0.02 + cornerNoiseD * 0.05), y: rightA },
                        { x: right, y: rightB },
                        { x: bottomA, y: bottom - uh * (0.01 + cornerNoiseE * 0.05) },
                        { x: bottomB, y: bottom },
                        { x: left + uw * (0.02 + cornerNoiseF * 0.04), y: leftA },
                        { x: left, y: leftB }
                    ]
                }

                function tracePolygon(ctx) {
                    const points = polygonPoints()
                    if (!points.length)
                        return
                    ctx.beginPath()
                    ctx.moveTo(points[0].x, points[0].y)
                    for (let pointIndex = 1; pointIndex < points.length; ++pointIndex)
                        ctx.lineTo(points[pointIndex].x, points[pointIndex].y)
                    ctx.closePath()
                }

                width: baseWidth + shardPad * 2.0
                height: baseHeight + shardPad * 2.0
                x: baseX - shardPad
                   + (randomA - 0.5) * baseWidth * 0.24
                   + normalX * (blastDistance + transientKick)
                   + tangentX * sideDrift * 1.22
                y: baseY - shardPad
                   + (randomB - 0.5) * baseHeight * 0.24
                   + normalY * (blastDistance + transientKick)
                   + tangentY * sideDrift * 1.22
                   + hover
                rotation:
                    (randomA - 0.5)
                    * (24.0 + activation * 156.0)
                    + Math.sin(
                        root.phase * (0.45 + randomB)
                        + randomC * Math.PI * 4.0)
                      * activation * 16.0
                scale:
                    1.0
                    + activation * (0.04 + randomC * 0.22)
                opacity:
                    Math.min(
                        1.0,
                        0.12
                        + root.fragmentProgress * 1.55
                        + activation * 0.32)
                transformOrigin: Item.Center
                Item {
                    id: shardSource

                    anchors.fill: parent
                    visible: false

                    Image {
                        anchors.fill: parent
                        source: root.artworkIllustrationSource
                        sourceSize.width: root.sourcePixelWidth
                        sourceSize.height: root.sourcePixelHeight
                        sourceClipRect: Qt.rect(
                            shard.sourceX,
                            shard.sourceY,
                            shard.sourceWidth,
                            shard.sourceHeight)
                        fillMode: Image.Stretch
                        smooth: true
                        mipmap: true
                        cache: true
                        asynchronous: true
                    }
                }

                Canvas {
                    id: shardMask

                    anchors.fill: parent
                    visible: false
                    antialiasing: true
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.clearRect(0, 0, width, height)
                        ctx.fillStyle = "white"
                        shard.tracePolygon(ctx)
                        ctx.fill()
                    }
                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                    Component.onCompleted: requestPaint()
                }

                OpacityMask {
                    anchors.fill: parent
                    source: shardSource
                    maskSource: shardMask
                    cached: true
                }

                Canvas {
                    id: shardOutline

                    anchors.fill: parent
                    visible: shard.activation > 0.10
                    antialiasing: true
                    opacity: Math.min(1.0, shard.activation * 0.92)
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = root.ink(0.12 + shard.activation * 0.18)
                        ctx.lineWidth = 0.8
                        shard.tracePolygon(ctx)
                        ctx.stroke()
                    }
                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                    Component.onCompleted: requestPaint()
                }
            }
        }
    }

    Canvas {
        id: motionCanvas

        anchors.fill: parent
        visible: !root.artworkReady
        antialiasing: true
        opacity: 1.0
        onPaint: root.paintScene(getContext("2d"))
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        visible: root.artworkAvailable && !root.artworkReady
        text: "正在读取专辑视觉"
        color: root.ink(0.32)
        font.pixelSize: 11
    }

    onSeedChanged: motionCanvas.requestPaint()
    onTemplateIndexChanged: motionCanvas.requestPaint()
    onArtworkReadyChanged: motionCanvas.requestPaint()
    onArtworkFocalXChanged: motionCanvas.requestPaint()
    onArtworkFocalYChanged: motionCanvas.requestPaint()
    onArtworkEdgeDensityChanged: motionCanvas.requestPaint()
    onArtworkContrastChanged: motionCanvas.requestPaint()
    onArtworkParticlesChanged: motionCanvas.requestPaint()
    onBassChanged: motionCanvas.requestPaint()
    onMidChanged: motionCanvas.requestPaint()
    onHighChanged: motionCanvas.requestPaint()
    onTransientLevelChanged: motionCanvas.requestPaint()
    onReducedMotionChanged: {
        if (reducedMotion) {
            explosion = 0.0
            pitchMemory = 0.0
        }
        motionCanvas.requestPaint()
    }
}
