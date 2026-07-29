#!/usr/bin/env python3
# Apply Aurora Controls Identity Pack 02 — HQ vector refinement.
#
# Requires Pack 01 markers in qml/MusicSpace.qml.

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys


class UpdateError(RuntimeError):
    pass


INLINE_COMPONENTS = r'''
    // AUR-CONTROLS-IDENTITY-PACK-01:BEGIN
    component SharinganTransportButton: Item {
        id: eyeButton

        property int direction: -1
        property color inkColor: "#171416"
        property color fillColor: "#fbf6eb"
        property bool reducedMotion: false
        property bool armed: true
        property real blinkProgress: 0.0
        property real irisPhase: 0.0
        property real pulsePhase: 0.0
        property real hoverAmount: eyeHover.hovered ? 1.0 : 0.0
        property real pressAmount: eyeTap.pressed ? 1.0 : 0.0

        signal activated()

        width: 82
        height: 58
        opacity: armed ? 1.0 : 0.44
        scale: 1.0 + hoverAmount * 0.035 - pressAmount * 0.025
        rotation: direction < 0 ? -1.2 : 1.2

        function doBlinkAndActivate() {
            if (!armed || clickBlink.running)
                return
            clickBlink.restart()
        }

        function requestRepaint() {
            eyeCanvas.requestPaint()
        }

        onBlinkProgressChanged: requestRepaint()
        onIrisPhaseChanged: requestRepaint()
        onPulsePhaseChanged: requestRepaint()
        onHoverAmountChanged: requestRepaint()
        onPressAmountChanged: requestRepaint()
        onDirectionChanged: requestRepaint()
        onInkColorChanged: requestRepaint()
        onFillColorChanged: requestRepaint()

        Timer {
            id: idleBlinkTimer
            interval: 2600
            repeat: true
            running: eyeButton.visible && eyeButton.opacity > 0.01
            onTriggered: {
                if (!clickBlink.running && !idleBlink.running)
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
            PauseAnimation { duration: 48 }
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 175
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
            PauseAnimation { duration: 28 }
            ScriptAction { script: eyeButton.activated() }
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 185
                easing.type: Easing.OutBack
            }
        }

        NumberAnimation on irisPhase {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
            running: eyeButton.visible && !eyeButton.reducedMotion
        }

        NumberAnimation on pulsePhase {
            from: 0
            to: Math.PI * 2.0
            duration: 2400
            loops: Animation.Infinite
            running: eyeButton.visible && !eyeButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation { duration: 130; easing.type: Easing.OutCubic }
        }

        HoverHandler { id: eyeHover }

        TapHandler {
            id: eyeTap
            enabled: eyeButton.armed
            onTapped: eyeButton.doBlinkAndActivate()
        }

        Canvas {
            id: eyeCanvas

            anchors.centerIn: parent
            width: parent.width * 2.0
            height: parent.height * 2.0
            scale: 0.5
            transformOrigin: Item.Center
            antialiasing: true
            renderTarget: Canvas.Image
            renderStrategy: Canvas.Immediate

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                const cx = w * 0.50
                const cy = h * 0.48
                const open = 1.0 - eyeButton.blinkProgress
                const baseEyeHeight = h * 0.54
                const eyeHeight = Math.max(h * 0.025, baseEyeHeight * open)
                const left = w * 0.055
                const right = w * 0.945
                const top = cy - eyeHeight * 0.50
                const bottom = cy + eyeHeight * 0.50

                ctx.save()

                ctx.shadowBlur = 7.0
                ctx.shadowColor = "rgba(17,12,15,0.16)"
                ctx.beginPath()
                ctx.moveTo(left, cy)
                ctx.bezierCurveTo(
                    w * 0.24, top - h * 0.035,
                    w * 0.72, top - h * 0.02,
                    right, cy)
                ctx.bezierCurveTo(
                    w * 0.72, bottom + h * 0.035,
                    w * 0.24, bottom + h * 0.025,
                    left, cy)
                ctx.closePath()

                const sclera = ctx.createLinearGradient(0, top, 0, bottom)
                sclera.addColorStop(0.0, "#fffdf8")
                sclera.addColorStop(0.55, eyeButton.fillColor)
                sclera.addColorStop(1.0, "#e8dfd2")
                ctx.fillStyle = sclera
                ctx.fill()
                ctx.shadowBlur = 0
                ctx.lineWidth = 3.5
                ctx.strokeStyle = eyeButton.inkColor
                ctx.stroke()
                ctx.clip()

                if (open > 0.08) {
                    const irisRadius = Math.max(6.0, h * 0.205 * open)
                    const irisX = cx + eyeButton.direction * w * 0.012
                    const irisY = cy
                    const pulse = 1.0 + Math.sin(eyeButton.pulsePhase) * 0.025

                    const irisGradient = ctx.createRadialGradient(
                        irisX - irisRadius * 0.28,
                        irisY - irisRadius * 0.30,
                        irisRadius * 0.12,
                        irisX,
                        irisY,
                        irisRadius * pulse)
                    irisGradient.addColorStop(0.0, "#ff8b86")
                    irisGradient.addColorStop(0.32, "#e54442")
                    irisGradient.addColorStop(0.72, "#9a121d")
                    irisGradient.addColorStop(1.0, "#3b0710")

                    ctx.beginPath()
                    ctx.arc(irisX, irisY, irisRadius * pulse, 0, Math.PI * 2.0)
                    ctx.fillStyle = irisGradient
                    ctx.fill()
                    ctx.lineWidth = 2.2
                    ctx.strokeStyle = "#1a0c10"
                    ctx.stroke()

                    ctx.beginPath()
                    ctx.arc(irisX, irisY, irisRadius * 0.72, 0, Math.PI * 2.0)
                    ctx.lineWidth = 1.4
                    ctx.strokeStyle = "rgba(32,7,12,0.78)"
                    ctx.stroke()

                    ctx.save()
                    ctx.translate(irisX, irisY)
                    ctx.rotate(
                        eyeButton.irisPhase
                        * Math.PI / 180.0
                        * (eyeButton.direction > 0 ? 1.0 : -1.0))

                    for (let i = 0; i < 3; ++i) {
                        ctx.save()
                        ctx.rotate(i * Math.PI * 2.0 / 3.0)

                        const tomoeY = -irisRadius * 0.61
                        const tomoeR = irisRadius * 0.145

                        ctx.beginPath()
                        ctx.arc(0, tomoeY, tomoeR, 0, Math.PI * 2.0)
                        ctx.fillStyle = "#11080b"
                        ctx.fill()

                        ctx.beginPath()
                        ctx.moveTo(tomoeR * 0.45, tomoeY + tomoeR * 0.20)
                        ctx.quadraticCurveTo(
                            tomoeR * 1.75,
                            tomoeY + tomoeR * 1.15,
                            tomoeR * 0.72,
                            tomoeY + tomoeR * 2.30)
                        ctx.quadraticCurveTo(
                            tomoeR * 0.35,
                            tomoeY + tomoeR * 1.25,
                            -tomoeR * 0.15,
                            tomoeY + tomoeR * 0.82)
                        ctx.closePath()
                        ctx.fill()
                        ctx.restore()
                    }
                    ctx.restore()

                    ctx.beginPath()
                    ctx.arc(irisX, irisY, irisRadius * 0.285, 0, Math.PI * 2.0)
                    ctx.fillStyle = "#080508"
                    ctx.fill()

                    ctx.beginPath()
                    ctx.arc(
                        irisX - irisRadius * 0.31,
                        irisY - irisRadius * 0.35,
                        irisRadius * 0.12,
                        0,
                        Math.PI * 2.0)
                    ctx.fillStyle = "rgba(255,255,255,0.72)"
                    ctx.fill()
                }

                ctx.restore()

                ctx.beginPath()
                ctx.moveTo(left - w * 0.005, cy)
                ctx.bezierCurveTo(
                    w * 0.23, top - h * 0.045,
                    w * 0.74, top - h * 0.030,
                    right + w * 0.005, cy)
                ctx.lineWidth = 5.2
                ctx.lineCap = "round"
                ctx.strokeStyle = eyeButton.inkColor
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(left + w * 0.01, cy + h * 0.006)
                ctx.bezierCurveTo(
                    w * 0.25, bottom + h * 0.030,
                    w * 0.72, bottom + h * 0.025,
                    right - w * 0.01, cy + h * 0.006)
                ctx.lineWidth = 2.2
                ctx.strokeStyle = "rgba(31,24,27,0.72)"
                ctx.stroke()

                if (eyeButton.blinkProgress > 0.82) {
                    ctx.beginPath()
                    ctx.moveTo(left + w * 0.06, cy)
                    ctx.quadraticCurveTo(cx, cy + h * 0.035, right - w * 0.06, cy)
                    ctx.lineWidth = 3.4
                    ctx.strokeStyle = eyeButton.inkColor
                    ctx.stroke()
                }

                const arrowY = h * 0.90
                const arrowCenter = cx
                const sign = eyeButton.direction
                ctx.beginPath()
                ctx.moveTo(arrowCenter - sign * w * 0.085, arrowY)
                ctx.lineTo(arrowCenter + sign * w * 0.085, arrowY)
                ctx.lineTo(
                    arrowCenter + sign * w * 0.045,
                    arrowY - h * 0.045)
                ctx.moveTo(arrowCenter + sign * w * 0.085, arrowY)
                ctx.lineTo(
                    arrowCenter + sign * w * 0.045,
                    arrowY + h * 0.045)
                ctx.lineWidth = 2.1
                ctx.lineCap = "round"
                ctx.strokeStyle = "rgba(25,19,22,0.60)"
                ctx.stroke()
            }
        }
    }

    component SupercarPlayToggle: Item {
        id: supercarButton

        property bool playing: false
        property bool hasTrack: false
        property color inkColor: "#171416"
        property color fillColor: "#fbf6eb"
        property color accentColor: "#8797d6"
        property bool reducedMotion: false
        property real wheelPhase: 0.0
        property real drivePhase: 0.0
        property real suspensionPhase: 0.0
        property real hoverAmount: carHover.hovered ? 1.0 : 0.0
        property real pressAmount: carTap.pressed ? 1.0 : 0.0

        signal activated()

        width: 112
        height: 88
        scale: 1.0 + hoverAmount * 0.035 - pressAmount * 0.025

        function requestRepaint() {
            carCanvas.requestPaint()
        }

        onWheelPhaseChanged: requestRepaint()
        onDrivePhaseChanged: requestRepaint()
        onSuspensionPhaseChanged: requestRepaint()
        onHoverAmountChanged: requestRepaint()
        onPressAmountChanged: requestRepaint()
        onPlayingChanged: requestRepaint()
        onHasTrackChanged: requestRepaint()
        onAccentColorChanged: requestRepaint()
        onInkColorChanged: requestRepaint()
        onFillColorChanged: requestRepaint()

        NumberAnimation on wheelPhase {
            from: 0
            to: 360
            duration: 620
            loops: Animation.Infinite
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        NumberAnimation on drivePhase {
            from: 0
            to: 1
            duration: 760
            loops: Animation.Infinite
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        NumberAnimation on suspensionPhase {
            from: -1
            to: 1
            duration: 980
            loops: Animation.Infinite
            easing.type: Easing.InOutSine
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation { duration: 130; easing.type: Easing.OutCubic }
        }

        HoverHandler { id: carHover }

        TapHandler {
            id: carTap
            onTapped: supercarButton.activated()
        }

        Canvas {
            id: carCanvas

            anchors.centerIn: parent
            width: parent.width * 2.0
            height: parent.height * 2.0
            scale: 0.5
            transformOrigin: Item.Center
            antialiasing: true
            renderTarget: Canvas.Image
            renderStrategy: Canvas.Immediate

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                const cx = w * 0.50
                const carLift = supercarButton.playing
                        ? supercarButton.suspensionPhase * h * 0.012
                        : 0.0
                const roadY = h * 0.78
                const bodyY = h * 0.48 + carLift
                const enabledAlpha = supercarButton.hasTrack ? 1.0 : 0.58

                ctx.save()
                ctx.translate(cx, roadY + h * 0.035)
                ctx.scale(w * 0.31, h * 0.055)
                ctx.beginPath()
                ctx.arc(0, 0, 1, 0, Math.PI * 2.0)
                ctx.fillStyle = "rgba(18,15,17,0.10)"
                ctx.fill()
                ctx.restore()

                ctx.save()
                ctx.translate(cx, h * 0.49)
                ctx.scale(w * 0.43, h * 0.40)
                ctx.beginPath()
                ctx.arc(0, 0, 1, 0, Math.PI * 2.0)
                ctx.restore()

                const pedestal = ctx.createLinearGradient(0, h * 0.15, 0, h * 0.90)
                pedestal.addColorStop(0.0, "rgba(255,253,248,0.97)")
                pedestal.addColorStop(1.0, "rgba(229,220,207,0.92)")
                ctx.save()
                ctx.translate(cx, h * 0.49)
                ctx.scale(w * 0.43, h * 0.40)
                ctx.beginPath()
                ctx.arc(0, 0, 1, 0, Math.PI * 2.0)
                ctx.restore()
                ctx.fillStyle = pedestal
                ctx.fill()
                ctx.lineWidth = 3.2
                ctx.strokeStyle = supercarButton.inkColor
                ctx.stroke()

                if (supercarButton.playing) {
                    for (let line = 0; line < 4; ++line) {
                        const phase = (supercarButton.drivePhase + line * 0.24) % 1.0
                        const startX = w * (0.12 + phase * 0.22)
                        const length = w * (0.06 + line * 0.018)
                        ctx.beginPath()
                        ctx.moveTo(startX, roadY + line * 3.0)
                        ctx.lineTo(startX - length, roadY + line * 3.0)
                        ctx.lineWidth = 2.0
                        ctx.strokeStyle = "rgba(42,38,42,0.22)"
                        ctx.stroke()
                    }
                }

                ctx.save()
                ctx.translate(0, carLift)

                const left = w * 0.18
                const right = w * 0.82
                const top = h * 0.34
                const bottom = h * 0.68

                ctx.shadowBlur = supercarButton.playing ? 10.0 : 5.0
                ctx.shadowColor = "rgba(18,14,17,0.24)"

                ctx.beginPath()
                ctx.moveTo(left, bottom - h * 0.06)
                ctx.lineTo(left + w * 0.055, top + h * 0.12)
                ctx.lineTo(left + w * 0.18, top + h * 0.055)
                ctx.lineTo(left + w * 0.30, top)
                ctx.lineTo(right - w * 0.20, top + h * 0.01)
                ctx.lineTo(right - w * 0.09, top + h * 0.10)
                ctx.lineTo(right, bottom - h * 0.10)
                ctx.lineTo(right - w * 0.025, bottom)
                ctx.lineTo(left + w * 0.06, bottom)
                ctx.closePath()

                const bodyGradient = ctx.createLinearGradient(0, top, 0, bottom)
                bodyGradient.addColorStop(0.0, "#38353a")
                bodyGradient.addColorStop(0.45, "#17161a")
                bodyGradient.addColorStop(1.0, "#07070a")
                ctx.fillStyle = bodyGradient
                ctx.fill()
                ctx.shadowBlur = 0
                ctx.lineWidth = 2.8
                ctx.strokeStyle = "#08070a"
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(left + w * 0.19, top + h * 0.065)
                ctx.lineTo(left + w * 0.31, top + h * 0.015)
                ctx.lineTo(right - w * 0.22, top + h * 0.025)
                ctx.lineTo(right - w * 0.12, top + h * 0.11)
                ctx.lineTo(left + w * 0.24, top + h * 0.12)
                ctx.closePath()

                const glassGradient = ctx.createLinearGradient(0, top, 0, top + h * 0.14)
                glassGradient.addColorStop(0.0, "rgba(205,220,230,0.72)")
                glassGradient.addColorStop(1.0, "rgba(61,75,85,0.82)")
                ctx.fillStyle = glassGradient
                ctx.fill()
                ctx.lineWidth = 1.5
                ctx.strokeStyle = "rgba(255,255,255,0.26)"
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(left + w * 0.045, bottom - h * 0.095)
                ctx.lineTo(left + w * 0.145, bottom - h * 0.13)
                ctx.lineTo(left + w * 0.17, bottom - h * 0.08)
                ctx.closePath()
                ctx.fillStyle = supercarButton.accentColor
                ctx.fill()

                ctx.beginPath()
                ctx.moveTo(right - w * 0.055, bottom - h * 0.12)
                ctx.lineTo(right - w * 0.155, bottom - h * 0.15)
                ctx.lineTo(right - w * 0.18, bottom - h * 0.09)
                ctx.closePath()
                ctx.fillStyle = "rgba(232,66,55,0.88)"
                ctx.fill()

                const wheelXs = [left + w * 0.15, right - w * 0.15]
                for (let i = 0; i < wheelXs.length; ++i) {
                    const wheelX = wheelXs[i]
                    const wheelY = bottom - h * 0.005
                    const radius = h * 0.105

                    ctx.beginPath()
                    ctx.arc(wheelX, wheelY, radius, 0, Math.PI * 2.0)
                    ctx.fillStyle = "#08080b"
                    ctx.fill()
                    ctx.lineWidth = 2.0
                    ctx.strokeStyle = "#2d2c31"
                    ctx.stroke()

                    ctx.beginPath()
                    ctx.arc(wheelX, wheelY, radius * 0.52, 0, Math.PI * 2.0)
                    ctx.fillStyle = "#65636a"
                    ctx.fill()

                    ctx.save()
                    ctx.translate(wheelX, wheelY)
                    ctx.rotate(supercarButton.wheelPhase * Math.PI / 180.0)
                    ctx.strokeStyle = "rgba(245,245,248,0.72)"
                    ctx.lineWidth = 1.8
                    for (let spoke = 0; spoke < 6; ++spoke) {
                        ctx.rotate(Math.PI / 3.0)
                        ctx.beginPath()
                        ctx.moveTo(0, 0)
                        ctx.lineTo(0, -radius * 0.48)
                        ctx.stroke()
                    }
                    ctx.restore()

                    ctx.beginPath()
                    ctx.arc(wheelX, wheelY, radius * 0.15, 0, Math.PI * 2.0)
                    ctx.fillStyle = supercarButton.accentColor
                    ctx.fill()
                }

                if (supercarButton.playing) {
                    const flame = 0.55 + Math.sin(supercarButton.drivePhase * Math.PI * 2.0) * 0.18
                    ctx.beginPath()
                    ctx.moveTo(right - w * 0.01, bottom - h * 0.09)
                    ctx.lineTo(right + w * (0.05 + flame * 0.04), bottom - h * 0.13)
                    ctx.lineTo(right + w * (0.04 + flame * 0.035), bottom - h * 0.05)
                    ctx.closePath()
                    ctx.fillStyle = "rgba(113,135,231,0.42)"
                    ctx.fill()
                }

                ctx.restore()

                if (!supercarButton.playing) {
                    ctx.beginPath()
                    ctx.moveTo(cx - w * 0.025, h * 0.23)
                    ctx.lineTo(cx - w * 0.025, h * 0.28)
                    ctx.moveTo(cx + w * 0.025, h * 0.23)
                    ctx.lineTo(cx + w * 0.025, h * 0.28)
                    ctx.lineWidth = 3.0
                    ctx.strokeStyle = "rgba(27,22,25,0.58)"
                    ctx.stroke()
                }
            }
        }
    }
    // AUR-CONTROLS-IDENTITY-PACK-01:END
'''


CONTROL_BLOCK = r'''
            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-BEGIN
            Item {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: playerContent.wide ? 2 : 0
                width: 310
                height: 112
                z: 80

                SharinganTransportButton {
                    id: previousSharinganButton
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed: root.controlsVisible && !liquidTrackTransition.running
                    reducedMotion: root.reducedMotion
                    direction: -1
                    inkColor: root.mangaButtonBorder
                    fillColor: root.mangaButtonFill
                    onActivated: root.beginTrackTransition(-1)
                }

                SupercarPlayToggle {
                    id: supercarTransportButton
                    anchors.centerIn: parent
                    width: root.controlsVisible ? 116 : 88
                    height: width * 0.80
                    opacity: root.controlsVisible ? 1.0 : 0.94
                    playing: AudioRuntime.playing
                    hasTrack: AudioRuntime.hasTrack
                    reducedMotion: root.reducedMotion
                    inkColor: root.mangaButtonBorder
                    fillColor: root.mangaButtonFill
                    accentColor: root.displayIdentityColor
                    onActivated: {
                        if (AudioRuntime.hasTrack)
                            AudioRuntime.togglePlayback()
                        else
                            audioDialog.open()
                    }
                }

                SharinganTransportButton {
                    id: nextSharinganButton
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed: root.controlsVisible && !liquidTrackTransition.running
                    reducedMotion: root.reducedMotion
                    direction: 1
                    inkColor: root.mangaButtonBorder
                    fillColor: root.mangaButtonFill
                    onActivated: root.beginTrackTransition(1)
                }
            }
            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-END
'''


def replace_marked_block(
    text: str,
    begin_marker: str,
    end_marker: str,
    replacement: str,
) -> str:
    if begin_marker not in text or end_marker not in text:
        raise UpdateError(
            "Pack 01 markers were not found. Apply Pack 01 before Pack 02."
        )

    pattern = re.compile(
        re.escape(begin_marker)
        + r".*?"
        + re.escape(end_marker),
        re.S,
    )
    return pattern.sub(replacement.strip("\n"), text, count=1)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("repository", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    root = args.repository.expanduser().resolve()
    qml_path = root / "qml" / "MusicSpace.qml"

    try:
        if not (root / "CMakeLists.txt").is_file():
            raise UpdateError("not an Aurora repository root")
        if not qml_path.is_file():
            raise UpdateError("missing qml/MusicSpace.qml")

        original = qml_path.read_text(encoding="utf-8")

        updated = replace_marked_block(
            original,
            "    // AUR-CONTROLS-IDENTITY-PACK-01:BEGIN",
            "    // AUR-CONTROLS-IDENTITY-PACK-01:END",
            INLINE_COMPONENTS,
        )
        updated = replace_marked_block(
            updated,
            "            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-BEGIN",
            "            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-END",
            CONTROL_BLOCK,
        )

    except (OSError, UnicodeError, UpdateError) as error:
        print(f"Controls HQ refinement failed: {error}", file=sys.stderr)
        return 1

    if args.check:
        if updated == original:
            print("Controls Identity Pack 02 is already applied.")
        else:
            print("Controls Identity Pack 02 applicability check passed.")
            print("would update: qml/MusicSpace.qml")
        return 0

    if updated == original:
        print("No files changed; Controls Identity Pack 02 is already applied.")
        return 0

    qml_path.write_text(updated, encoding="utf-8")
    print("updated: qml/MusicSpace.qml")
    print("Controls Identity Pack 02 applied successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
