import QtQuick
import QtQuick.Window
import Aurora.Runtime 1.0

Window {
    id: window

    property bool presentationMode:
        Qt.application.arguments.indexOf("--presentation") >= 0

    width: 1360
    height: 840
    minimumWidth: 920
    minimumHeight: 640
    visible: true
    color: AuroraTokens.mangaPaper
    title: AuroraI18n.traditionalChinese
           ? "Aurora 音樂記憶空間 · Demo 1.0"
           : "Aurora Music Memory Space · Demo 1.0"

    Component.onCompleted: {
        if (presentationMode)
            showFullScreen()
    }

    function setPresentationMode(enabled) {
        presentationMode = enabled
        if (enabled)
            showFullScreen()
        else
            showNormal()
    }

    Shortcut {
        sequence: "F11"
        onActivated: window.setPresentationMode(!window.presentationMode)
    }

    Shortcut {
        sequence: "Esc"
        enabled: window.presentationMode
        onActivated: window.setPresentationMode(false)
    }

    AppShell {
        anchors.fill: parent
        presentationMode: window.presentationMode
    }
}
