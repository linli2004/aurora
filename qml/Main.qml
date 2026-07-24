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
    color: AuroraTokens.mangaPaper
    title: AuroraI18n.traditionalChinese
           ? "Aurora 音樂記憶空間 · Demo"
           : "Aurora Music Memory Space · Demo"

    AppShell { anchors.fill: parent }
}
