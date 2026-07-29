import QtQuick

Item {
    id: root

    property real pointX: 0.0
    property real pointY: 0.0
    property real velocityX: 1.0
    property real velocityY: 0.0
    property real depth: 0.0
    property real networkOpacity: 1.0
    property color colorSignature: AuroraTokens.coolAccent

    readonly property real headX: width * 0.72
    readonly property real headY: height * 0.50
    readonly property real meteorAngle:
        Math.abs(velocityX) + Math.abs(velocityY) > 0.001
        ? Math.atan2(velocityY, velocityX) * 180.0 / Math.PI
        : 0.0

    visible: networkOpacity > 0.01
    x: pointX - headX
    y: pointY - headY
    width: 178 + Math.max(0.0, depth) * 44
    height: 78 + Math.max(0.0, depth) * 18
    opacity: networkOpacity
    z: Math.round(220 + depth * 40)
    transform: Rotation {
        origin.x: root.headX
        origin.y: root.headY
        angle: root.meteorAngle
    }

    Canvas {
        id: meteorCanvas

        anchors.fill: parent
        antialiasing: true
        renderTarget: Canvas.Image

        function rgba(alpha) {
            return "rgba("
                    + Math.round(root.colorSignature.r * 255) + ","
                    + Math.round(root.colorSignature.g * 255) + ","
                    + Math.round(root.colorSignature.b * 255) + ","
                    + alpha + ")"
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            const headX = root.headX
            const headY = root.headY
            const coreRadius = Math.min(width, height) * 0.18
            const tailStart = width * 0.05
            const gradient = ctx.createLinearGradient(tailStart, headY, headX, headY)
            gradient.addColorStop(0.0, "rgba(255,255,255,0.0)")
            gradient.addColorStop(0.22, rgba(0.06))
            gradient.addColorStop(0.66, rgba(0.30))
            gradient.addColorStop(1.0, "rgba(255,255,255,0.64)")

            ctx.beginPath()
            ctx.moveTo(tailStart, headY)
            ctx.bezierCurveTo(width * 0.34, headY - height * 0.30,
                              width * 0.56, headY - height * 0.22,
                              headX, headY - coreRadius * 0.64)
            ctx.bezierCurveTo(width * 0.58, headY - height * 0.06,
                              width * 0.36, headY + height * 0.08,
                              tailStart, headY)
            ctx.closePath()
            ctx.fillStyle = gradient
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(tailStart + width * 0.10, headY + height * 0.10)
            ctx.bezierCurveTo(width * 0.38, headY + height * 0.28,
                              width * 0.56, headY + height * 0.20,
                              headX, headY + coreRadius * 0.58)
            ctx.lineWidth = 2.0
            ctx.strokeStyle = rgba(0.22)
            ctx.stroke()

            const glow = ctx.createRadialGradient(headX, headY, coreRadius * 0.15,
                                                  headX, headY, coreRadius * 2.4)
            glow.addColorStop(0.0, "rgba(255,255,255,0.84)")
            glow.addColorStop(0.18, rgba(0.72))
            glow.addColorStop(1.0, rgba(0.0))
            ctx.beginPath()
            ctx.arc(headX, headY, coreRadius * 2.4, 0, Math.PI * 2.0)
            ctx.fillStyle = glow
            ctx.fill()

            ctx.beginPath()
            ctx.arc(headX, headY, coreRadius * 0.82, 0, Math.PI * 2.0)
            ctx.fillStyle = rgba(0.86)
            ctx.fill()

            ctx.beginPath()
            ctx.arc(headX + coreRadius * 0.18, headY - coreRadius * 0.12,
                    coreRadius * 0.34, 0, Math.PI * 2.0)
            ctx.fillStyle = "rgba(255,255,255,0.88)"
            ctx.fill()
        }

        onVisibleChanged: requestPaint()
    }
}
