import QtQuick
import "MusicNetworkLayout.js" as NetworkLayout

Item {
    id: root

    property var nodes: []
    property string currentKey: ""
    property string targetKey: ""
    property color colorSignature: AuroraTokens.coolAccent
    property real networkOpacity: 1.0
    property real travelProgress: 0.0
    property real arrivedPulse: 0.0
    property real rotationPhase: 0.0

    readonly property var segments:
        NetworkLayout.buildSegments(nodes, currentKey, targetKey)
    readonly property var travelPathKeys:
        NetworkLayout.buildTravelPath(nodes, currentKey, targetKey)
    readonly property var currentNode: nodeByKey(currentKey)
    readonly property var targetNode: nodeByKey(targetKey)
    readonly property var travelPoint:
        pathPointAt(travelProgress)
    readonly property var previousTravelPoint:
        pathPointAt(Math.max(0.0, travelProgress - 0.026))
    readonly property real cameraFocus:
        clamp01(networkOpacity * (0.72 + travelProgress * 0.28))
    readonly property real cameraScale:
        1.0 + cameraFocus * 1.12
    readonly property real cameraX:
        width * 0.50 - travelPoint.x * cameraScale
    readonly property real cameraY:
        height * 0.50 - travelPoint.y * cameraScale
    readonly property real meteorVelocityX:
        travelPoint.x - previousTravelPoint.x
    readonly property real meteorVelocityY:
        travelPoint.y - previousTravelPoint.y

    function nodeByKey(key) {
        for (let index = 0; index < nodes.length; ++index) {
            if (nodes[index].key === key)
                return nodes[index]
        }
        return ({
            x: width * 0.38,
            y: height * 0.52,
            z: 0.0,
            sx: -0.55,
            sy: 0.12,
            sz: 0.65,
            key: ""
        })
    }

    function clamp01(value) {
        return Math.max(0.0, Math.min(1.0, value))
    }

    function vectorForNode(node) {
        return {
            x: node && node.sx !== undefined ? node.sx : 0.0,
            y: node && node.sy !== undefined ? node.sy : 0.0,
            z: node && node.sz !== undefined ? node.sz : 0.0
        }
    }

    function normalizeVector(vector) {
        const length = Math.max(0.001,
                                Math.sqrt(vector.x * vector.x
                                          + vector.y * vector.y
                                          + vector.z * vector.z))
        return {
            x: vector.x / length,
            y: vector.y / length,
            z: vector.z / length
        }
    }

    function spherePoint(fromNode, toNode, progress) {
        const eased = progress < 0.14
                ? 0.14 * Math.pow(progress / 0.14, 1.7)
                : progress > 0.82
                  ? 0.82 + 0.18 * (1.0 - Math.pow((1.0 - progress) / 0.18, 1.9))
                  : progress
        const from = vectorForNode(fromNode)
        const to = vectorForNode(toNode)
        return normalizeVector({
            x: from.x * (1.0 - eased) + to.x * eased,
            y: from.y * (1.0 - eased) + to.y * eased,
            z: from.z * (1.0 - eased) + to.z * eased
        })
    }

    function projectVector(vector) {
        const angleY = rotationPhase
        const angleX = -0.18
        const cosY = Math.cos(angleY)
        const sinY = Math.sin(angleY)
        const x1 = vector.x * cosY + vector.z * sinY
        const z1 = vector.z * cosY - vector.x * sinY
        const cosX = Math.cos(angleX)
        const sinX = Math.sin(angleX)
        const y2 = vector.y * cosX - z1 * sinX
        const z2 = z1 * cosX + vector.y * sinX
        const radius = Math.min(Math.max(width, 1), Math.max(height, 1)) * 0.48
        const perspective = 0.82 + z2 * 0.13
        return {
            x: width * 0.50 + x1 * radius * 1.18 * perspective,
            y: height * 0.48 + y2 * radius * 0.90 * perspective,
            z: z2
        }
    }

    function projectNode(node) {
        return projectVector(vectorForNode(node))
    }

    function projectedPathPoint(fromNode, toNode, progress) {
        return projectVector(spherePoint(fromNode, toNode, progress))
    }

    function pathPointAt(progress) {
        if (travelPathKeys.length < 2)
            return projectNode(currentNode)

        const byKey = {}
        for (let index = 0; index < nodes.length; ++index)
            byKey[nodes[index].key] = nodes[index]

        const segmentCount = travelPathKeys.length - 1
        const total = clamp01(progress) * segmentCount
        const segmentIndex = Math.min(segmentCount - 1, Math.floor(total))
        const localProgress = total - segmentIndex
        const fromNode = byKey[travelPathKeys[segmentIndex]] || currentNode
        const toNode = byKey[travelPathKeys[segmentIndex + 1]] || targetNode
        return projectedPathPoint(fromNode, toNode, localProgress)
    }

    function drawSphereGuides(ctx) {
        const radius = Math.min(Math.max(width, 1), Math.max(height, 1)) * 0.48
        const centerX = width * 0.50
        const centerY = height * 0.48

        ctx.save()
        ctx.translate(centerX, centerY)
        ctx.rotate(rotationPhase * 0.18)
        ctx.lineWidth = 0.9
        ctx.strokeStyle = "rgba(118,134,178," + 0.18 * networkOpacity + ")"

        ctx.beginPath()
        ctx.ellipse(0, 0, radius * 1.18, radius * 0.86, 0, 0, Math.PI * 2.0)
        ctx.stroke()

        ctx.beginPath()
        ctx.ellipse(0, 0, radius * 1.18, radius * 0.34, 0, 0, Math.PI * 2.0)
        ctx.stroke()

        ctx.beginPath()
        ctx.ellipse(0, 0, radius * 0.40, radius * 0.86, 0, 0, Math.PI * 2.0)
        ctx.stroke()

        ctx.restore()
    }

    onNodesChanged: lineCanvas.requestPaint()
    onCurrentKeyChanged: lineCanvas.requestPaint()
    onTargetKeyChanged: lineCanvas.requestPaint()
    onTravelProgressChanged: lineCanvas.requestPaint()
    onNetworkOpacityChanged: lineCanvas.requestPaint()
    onRotationPhaseChanged: lineCanvas.requestPaint()
    onWidthChanged: lineCanvas.requestPaint()
    onHeightChanged: lineCanvas.requestPaint()

    NumberAnimation on rotationPhase {
        from: 0.0
        to: 6.283185307
        duration: 28000
        loops: Animation.Infinite
        running: root.networkOpacity > 0.01
    }

    Item {
        id: cameraLayer

        width: root.width
        height: root.height
        x: root.cameraX
        y: root.cameraY
        scale: root.cameraScale
        transformOrigin: Item.TopLeft

        Canvas {
            id: lineCanvas

            anchors.fill: parent
            antialiasing: true
            renderTarget: Canvas.Image

            onPaint: {
                const ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                if (root.nodes.length < 2 || root.networkOpacity <= 0.01)
                    return

                root.drawSphereGuides(ctx)

                const byKey = {}
                for (let index = 0; index < root.nodes.length; ++index)
                    byKey[root.nodes[index].key] = root.nodes[index]

                for (let index = 0; index < root.segments.length; ++index) {
                    const segment = root.segments[index]
                    const fromNode = byKey[segment.from]
                    const toNode = byKey[segment.to]
                    if (!fromNode || !toNode)
                        continue

                    const active = segment.active === true
                    const fromPoint = root.projectNode(fromNode)
                    const toPoint = root.projectNode(toNode)

                    ctx.beginPath()
                    ctx.moveTo(fromPoint.x, fromPoint.y)
                    ctx.lineTo(toPoint.x, toPoint.y)
                    ctx.lineWidth = active ? 1.5 : 0.8
                    ctx.strokeStyle = active
                            ? "rgba(108,132,198," + 0.34 * root.networkOpacity + ")"
                            : "rgba(118,134,178," + 0.22 * root.networkOpacity + ")"
                    ctx.stroke()
                }

                if (root.travelPathKeys.length >= 2 && root.travelProgress > 0.02) {
                    const segmentCount = root.travelPathKeys.length - 1
                    const totalProgress = root.clamp01(root.travelProgress) * segmentCount
                    const completedSegment = Math.floor(totalProgress)
                    const byPathKey = byKey
                    for (let pathIndex = 0; pathIndex < segmentCount; ++pathIndex) {
                        const visibleAmount = pathIndex < completedSegment
                                ? 1.0
                                : pathIndex === completedSegment
                                  ? totalProgress - completedSegment
                                  : 0.0
                        if (visibleAmount <= 0.0)
                            continue

                        const fromNode = byPathKey[root.travelPathKeys[pathIndex]]
                        const toNode = byPathKey[root.travelPathKeys[pathIndex + 1]]
                        if (!fromNode || !toNode)
                            continue

                        const steps = 18
                        const visibleSteps = Math.max(1, Math.floor(steps * visibleAmount))
                        ctx.beginPath()
                        for (let step = 0; step <= visibleSteps; ++step) {
                            const point = root.projectedPathPoint(fromNode, toNode, step / steps)
                            if (step === 0)
                                ctx.moveTo(point.x, point.y)
                            else
                                ctx.lineTo(point.x, point.y)
                        }
                        ctx.lineWidth = 3.0
                        ctx.strokeStyle = "rgba(108,132,198," + 0.42 * root.networkOpacity + ")"
                        ctx.stroke()
                    }
                }
            }
        }

        Repeater {
            model: root.nodes

            MusicNetworkTransitionNode {
                node: modelData
                screenX: root.projectNode(modelData).x
                screenY: root.projectNode(modelData).y
                projectedDepth: root.projectNode(modelData).z
                colorSignature: root.colorSignature
                networkOpacity: root.networkOpacity
                travelProgress: root.travelProgress
                arrivedPulse: root.arrivedPulse
            }
        }

        MusicNetworkTravelLight {
            pointX: root.travelPoint.x
            pointY: root.travelPoint.y
            velocityX: root.meteorVelocityX
            velocityY: root.meteorVelocityY
            depth: root.travelPoint.z
            networkOpacity: root.networkOpacity
            colorSignature: root.colorSignature
        }
    }
}
