#!/usr/bin/env python3
"""Apply AUR controls identity pack 01.

Adds three control affordances to qml/MusicSpace.qml:
- sharingan-style previous button
- sharingan-style next button
- supercar-style play/pause button

The patch is intentionally text-based so it can apply on top of the user's
current Aurora repo without requiring an exact file hash match.
"""

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
        property color inkColor: "#1f1a1a"
        property color fillColor: "#f3eee2"
        property color accentColor: "#8897d8"
        property bool reducedMotion: false
        property bool armed: true
        property real blinkProgress: 0.0
        property real spinPhase: 0.0
        property real hoverAmount: eyeHover.hovered ? 1.0 : 0.0
        property real pressAmount: eyeTap.pressed ? 1.0 : 0.0

        signal activated()

        width: 58
        height: 44
        opacity: armed ? 1.0 : 0.42
        scale: 1.0 + hoverAmount * 0.03 - pressAmount * 0.02

        function doBlinkAndActivate() {
            if (!armed || clickBlink.running)
                return
            clickBlink.restart()
        }

        onBlinkProgressChanged: eyeCanvas.requestPaint()
        onSpinPhaseChanged: eyeCanvas.requestPaint()
        onHoverAmountChanged: eyeCanvas.requestPaint()
        onPressAmountChanged: eyeCanvas.requestPaint()
        onDirectionChanged: eyeCanvas.requestPaint()
        onAccentColorChanged: eyeCanvas.requestPaint()
        onInkColorChanged: eyeCanvas.requestPaint()
        onFillColorChanged: eyeCanvas.requestPaint()

        Timer {
            id: idleBlinkTimer
            interval: 2400
            repeat: true
            running: eyeButton.visible && eyeButton.opacity > 0.01
            onTriggered: {
                if (!clickBlink.running && !idleBlink.running)
                    idleBlink.restart()
                interval = 2200 + Math.floor(Math.random() * 1800)
            }
        }

        SequentialAnimation {
            id: idleBlink
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.95
                duration: 120
                easing.type: Easing.InCubic
            }
            PauseAnimation { duration: 42 }
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 180
                easing.type: Easing.OutCubic
            }
        }

        SequentialAnimation {
            id: clickBlink
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 1.0
                duration: 90
                easing.type: Easing.InCubic
            }
            ScriptAction { script: eyeButton.activated() }
            NumberAnimation {
                target: eyeButton
                property: "blinkProgress"
                to: 0.0
                duration: 160
                easing.type: Easing.OutCubic
            }
        }

        NumberAnimation on spinPhase {
            from: 0
            to: 360
            duration: 4200
            loops: Animation.Infinite
            running: eyeButton.visible && !eyeButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }

        HoverHandler { id: eyeHover }
        TapHandler {
            id: eyeTap
            enabled: eyeButton.armed
            onTapped: eyeButton.doBlinkAndActivate()
        }

        Canvas {
            id: eyeCanvas
            anchors.fill: parent
            antialiasing: true

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                const closed = eyeButton.blinkProgress
                const openHeight = h * 0.72
                const eyeHeight = Math.max(2.2, openHeight * (1.0 - 0.94 * closed))
                const cy = h * 0.50
                const top = cy - eyeHeight / 2
                const bottom = cy + eyeHeight / 2
                const left = w * 0.08
                const right = w * 0.92

                ctx.save()
                ctx.beginPath()
                ctx.moveTo(left, cy)
                ctx.quadraticCurveTo(w * 0.50, top, right, cy)
                ctx.quadraticCurveTo(w * 0.50, bottom, left, cy)
                ctx.closePath()
                ctx.fillStyle = eyeButton.fillColor
                ctx.fill()
                ctx.lineWidth = 2.0
                ctx.strokeStyle = eyeButton.inkColor
                ctx.stroke()
                ctx.clip()

                const irisRadius = Math.max(3.2, eyeHeight * 0.28)
                const pupilRadius = irisRadius * 0.42
                const irisX = w * 0.50 + eyeButton.direction * 2.0
                const irisY = cy

                const spinRad = eyeButton.spinPhase * Math.PI / 180.0
                const hoverBoost = 0.18 + eyeButton.hoverAmount * 0.08

                ctx.beginPath()
                ctx.arc(irisX, irisY, irisRadius, 0, Math.PI * 2)
                ctx.fillStyle = Qt.rgba(
                    Math.min(1.0, eyeButton.accentColor.r + hoverBoost),
                    eyeButton.accentColor.g * 0.72,
                    eyeButton.accentColor.b * 0.82,
                    1.0)
                ctx.fill()

                ctx.lineWidth = 1.2
                ctx.strokeStyle = eyeButton.inkColor
                ctx.stroke()

                ctx.save()
                ctx.translate(irisX, irisY)
                ctx.rotate(spinRad * (eyeButton.direction > 0 ? 1.0 : -1.0))
                for (let i = 0; i < 3; ++i) {
                    ctx.rotate((Math.PI * 2) / 3)
                    ctx.beginPath()
                    ctx.arc(0, -irisRadius * 0.55, irisRadius * 0.18, 0, Math.PI * 2)
                    ctx.fillStyle = eyeButton.inkColor
                    ctx.fill()
                    ctx.beginPath()
                    ctx.moveTo(0, -irisRadius * 0.18)
                    ctx.lineTo(0, -irisRadius * 0.62)
                    ctx.lineWidth = 1.0
                    ctx.strokeStyle = eyeButton.inkColor
                    ctx.stroke()
                }
                ctx.restore()

                ctx.beginPath()
                ctx.arc(irisX, irisY, pupilRadius, 0, Math.PI * 2)
                ctx.fillStyle = eyeButton.inkColor
                ctx.fill()

                ctx.beginPath()
                ctx.arc(irisX - irisRadius * 0.25, irisY - irisRadius * 0.28,
                        Math.max(0.9, irisRadius * 0.13), 0, Math.PI * 2)
                ctx.fillStyle = Qt.rgba(1, 1, 1, 0.55)
                ctx.fill()

                ctx.restore()

                ctx.lineWidth = 1.2
                ctx.strokeStyle = Qt.rgba(eyeButton.inkColor.r,
                                          eyeButton.inkColor.g,
                                          eyeButton.inkColor.b,
                                          0.45)
                ctx.beginPath()
                const hintY = h * 0.86
                const hintOffset = eyeButton.direction > 0 ? 1 : -1
                ctx.moveTo(w * 0.43, hintY)
                ctx.lineTo(w * 0.57, hintY)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(w * 0.57 - 5 * hintOffset, hintY - 4)
                ctx.lineTo(w * 0.57, hintY)
                ctx.lineTo(w * 0.57 - 5 * hintOffset, hintY + 4)
                ctx.stroke()
            }
        }
    }

    component SupercarPlayToggle: Item {
        id: supercarButton

        property bool playing: false
        property bool hasTrack: false
        property color inkColor: "#1f1a1a"
        property color fillColor: "#f3eee2"
        property color accentColor: "#8897d8"
        property bool reducedMotion: false
        property real wheelPhase: 0.0
        property real floatPhase: 0.0
        property real accentPulse: 0.0
        property real hoverAmount: carHover.hovered ? 1.0 : 0.0
        property real pressAmount: carTap.pressed ? 1.0 : 0.0

        signal activated()

        width: 86
        height: 86
        scale: 1.0 + hoverAmount * 0.03 - pressAmount * 0.02

        onWheelPhaseChanged: carCanvas.requestPaint()
        onFloatPhaseChanged: carCanvas.requestPaint()
        onAccentPulseChanged: carCanvas.requestPaint()
        onHoverAmountChanged: carCanvas.requestPaint()
        onPressAmountChanged: carCanvas.requestPaint()
        onPlayingChanged: carCanvas.requestPaint()
        onHasTrackChanged: carCanvas.requestPaint()
        onAccentColorChanged: carCanvas.requestPaint()
        onInkColorChanged: carCanvas.requestPaint()
        onFillColorChanged: carCanvas.requestPaint()

        NumberAnimation on wheelPhase {
            from: 0
            to: 360
            duration: 900
            loops: Animation.Infinite
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        NumberAnimation on floatPhase {
            from: -1
            to: 1
            duration: 1100
            loops: Animation.Infinite
            easing.type: Easing.InOutSine
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        NumberAnimation on accentPulse {
            from: 0.35
            to: 1.0
            duration: 720
            loops: Animation.Infinite
            easing.type: Easing.InOutSine
            running: supercarButton.playing && !supercarButton.reducedMotion
        }

        Behavior on scale {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }

        HoverHandler { id: carHover }
        TapHandler {
            id: carTap
            onTapped: supercarButton.activated()
        }

        Canvas {
            id: carCanvas
            anchors.fill: parent
            antialiasing: true

            onPaint: {
                const ctx = getContext("2d")
                const w = width
                const h = height
                ctx.clearRect(0, 0, w, h)

                const panelRadius = Math.min(w, h) * 0.40
                const panelAlpha = supercarButton.hasTrack ? 1.0 : 0.58
                const glowAlpha = supercarButton.playing ? (0.10 + supercarButton.accentPulse * 0.10) : 0.08
                const cx = w / 2
                const cy = h / 2

                ctx.beginPath()
                ctx.arc(cx, cy, panelRadius, 0, Math.PI * 2)
                ctx.fillStyle = Qt.rgba(supercarButton.fillColor.r,
                                        supercarButton.fillColor.g,
                                        supercarButton.fillColor.b,
                                        0.92 * panelAlpha)
                ctx.fill()
                ctx.lineWidth = 2.0
                ctx.strokeStyle = supercarButton.inkColor
                ctx.stroke()

                ctx.beginPath()
                ctx.arc(cx, cy, panelRadius - 8, 0, Math.PI * 2)
                ctx.strokeStyle = Qt.rgba(supercarButton.accentColor.r,
                                          supercarButton.accentColor.g,
                                          supercarButton.accentColor.b,
                                          glowAlpha)
                ctx.lineWidth = supercarButton.playing ? 3.0 : 1.6
                ctx.stroke()

                const bodyLift = supercarButton.playing ? supercarButton.floatPhase * 1.7 : 0.0
                const carY = cy + bodyLift + 2
                const carLeft = cx - 22
                const carTop = carY - 10

                if (supercarButton.playing) {
                    ctx.beginPath()
                    ctx.moveTo(cx + 16, carY)
                    ctx.lineTo(cx + 28 + supercarButton.accentPulse * 4, carY - 5)
                    ctx.lineTo(cx + 28 + supercarButton.accentPulse * 4, carY + 5)
                    ctx.closePath()
                    ctx.fillStyle = Qt.rgba(supercarButton.accentColor.r,
                                            supercarButton.accentColor.g,
                                            supercarButton.accentColor.b,
                                            0.25 + supercarButton.accentPulse * 0.18)
                    ctx.fill()
                }

                ctx.save()
                ctx.translate(0, bodyLift)
                ctx.beginPath()
                ctx.moveTo(carLeft, carTop + 18)
                ctx.lineTo(carLeft + 6, carTop + 8)
                ctx.lineTo(carLeft + 18, carTop + 4)
                ctx.lineTo(carLeft + 30, carTop + 5)
                ctx.lineTo(carLeft + 38, carTop + 12)
                ctx.lineTo(carLeft + 44, carTop + 14)
                ctx.lineTo(carLeft + 46, carTop + 18)
                ctx.lineTo(carLeft + 40, carTop + 20)
                ctx.lineTo(carLeft + 35, carTop + 24)
                ctx.lineTo(carLeft + 8, carTop + 24)
                ctx.lineTo(carLeft + 4, carTop + 21)
                ctx.closePath()
                ctx.fillStyle = Qt.rgba(supercarButton.inkColor.r,
                                        supercarButton.inkColor.g,
                                        supercarButton.inkColor.b,
                                        supercarButton.hasTrack ? 0.98 : 0.62)
                ctx.fill()

                ctx.beginPath()
                ctx.moveTo(carLeft + 12, carTop + 9)
                ctx.lineTo(carLeft + 20, carTop + 6)
                ctx.lineTo(carLeft + 30, carTop + 7)
                ctx.lineTo(carLeft + 34, carTop + 13)
                ctx.lineTo(carLeft + 18, carTop + 13)
                ctx.closePath()
                ctx.fillStyle = Qt.rgba(1, 1, 1, 0.18)
                ctx.fill()

                const wheelCenters = [carLeft + 13, carLeft + 34]
                for (let i = 0; i < wheelCenters.length; ++i) {
                    const wheelX = wheelCenters[i]
                    const wheelY = carTop + 24
                    ctx.beginPath()
                    ctx.arc(wheelX, wheelY, 6.7, 0, Math.PI * 2)
                    ctx.fillStyle = Qt.rgba(0.06, 0.07, 0.10, supercarButton.hasTrack ? 1.0 : 0.55)
                    ctx.fill()
                    ctx.beginPath()
                    ctx.arc(wheelX, wheelY, 3.1, 0, Math.PI * 2)
                    ctx.fillStyle = Qt.rgba(1, 1, 1, 0.22)
                    ctx.fill()

                    if (supercarButton.playing) {
                        ctx.save()
                        ctx.translate(wheelX, wheelY)
                        ctx.rotate(supercarButton.wheelPhase * Math.PI / 180.0)
                        ctx.lineWidth = 1.0
                        ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.22)
                        ctx.beginPath()
                        ctx.moveTo(-3.5, 0)
                        ctx.lineTo(3.5, 0)
                        ctx.moveTo(0, -3.5)
                        ctx.lineTo(0, 3.5)
                        ctx.stroke()
                        ctx.restore()
                    }
                }
                ctx.restore()
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
                anchors.bottomMargin: playerContent.wide ? 8 : 0
                width: 236
                height: 92

                SharinganTransportButton {
                    id: previousSharinganButton
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed: root.controlsVisible && !liquidTrackTransition.running
                    reducedMotion: root.reducedMotion
                    direction: -1
                    inkColor: root.mangaButtonBorder
                    fillColor: root.mangaButtonFill
                    accentColor: root.displayIdentityColor
                    onActivated: root.beginTrackTransition(-1)
                }

                SupercarPlayToggle {
                    id: supercarTransportButton
                    anchors.centerIn: parent
                    width: root.controlsVisible ? 88 : 68
                    height: width
                    opacity: root.controlsVisible ? 1.0 : 0.92
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
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.controlOpacity
                    armed: root.controlsVisible && !liquidTrackTransition.running
                    reducedMotion: root.reducedMotion
                    direction: 1
                    inkColor: root.mangaButtonBorder
                    fillColor: root.mangaButtonFill
                    accentColor: root.displayIdentityColor
                    onActivated: root.beginTrackTransition(1)
                }
            }
            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-END
'''


def replace_transport_block(text: str) -> str:
    current_begin = '            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-BEGIN'
    current_end = '            // AUR-CONTROLS-IDENTITY-PACK-01:TRANSPORT-END'
    if current_begin in text and current_end in text:
        pattern = re.compile(re.escape(current_begin) + r'.*?' + re.escape(current_end), re.S)
        return pattern.sub(CONTROL_BLOCK.strip('\n'), text, count=1)

    start_marker = '            Row {\n                anchors.left: parent.left\n                anchors.bottom: parent.bottom\n                anchors.bottomMargin: playerContent.wide ? 8 : 0\n                spacing: 14\n'
    behavior_marker = '\n            Behavior on x {'
    start = text.find(start_marker)
    if start == -1:
        raise UpdateError('could not locate the existing transport control row')
    end = text.find(behavior_marker, start)
    if end == -1:
        raise UpdateError('could not locate the end of the existing transport control row')
    return text[:start] + CONTROL_BLOCK + text[end:]


def insert_or_replace_inline_components(text: str) -> str:
    begin = '    // AUR-CONTROLS-IDENTITY-PACK-01:BEGIN'
    end = '    // AUR-CONTROLS-IDENTITY-PACK-01:END'
    if begin in text and end in text:
        pattern = re.compile(re.escape(begin) + r'.*?' + re.escape(end), re.S)
        return pattern.sub(INLINE_COMPONENTS.strip('\n'), text, count=1)

    marker = '    NumberAnimation on presencePhase {'
    index = text.find(marker)
    if index == -1:
        raise UpdateError('could not find an insertion point for the new control components')
    return text[:index] + INLINE_COMPONENTS + '\n' + text[index:]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('repository', type=Path)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()

    root = args.repository.expanduser().resolve()
    qml_path = root / 'qml' / 'MusicSpace.qml'

    try:
        if not (root / 'CMakeLists.txt').is_file():
            raise UpdateError('not an Aurora repository root')
        if not qml_path.is_file():
            raise UpdateError('missing qml/MusicSpace.qml')

        original = qml_path.read_text(encoding='utf-8')
        updated = insert_or_replace_inline_components(original)
        updated = replace_transport_block(updated)
    except (OSError, UnicodeError, UpdateError) as exc:
        print(f'Controls identity pack failed: {exc}', file=sys.stderr)
        return 1

    if args.check:
        if updated == original:
            print('Controls identity pack is already applied.')
        else:
            print('Controls identity pack applicability check passed.')
            print('would update: qml/MusicSpace.qml')
        return 0

    if updated != original:
        qml_path.write_text(updated, encoding='utf-8')
        print('updated: qml/MusicSpace.qml')
        print('Controls identity pack applied successfully.')
    else:
        print('No files changed; the controls identity pack was already applied.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
