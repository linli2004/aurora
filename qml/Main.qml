import QtQuick
import QtQuick.Window
import Aurora.Runtime 1.0

Window {
    id: window
    width: 1280
    height: 820
    minimumWidth: 920
    minimumHeight: 640
    visible: true
    color: AuroraTokens.windowBackground
    title: "Aurora Music Framework — Prototype 0.5.2"

    AppShell { anchors.fill: parent }
}
