import QtQuick

Rectangle {
    id: root

    signal toggled()

    width: 92
    height: 38
    radius: 8
    color: AuroraTokens.mangaPaper
    border.width: 2
    border.color: AuroraTokens.mangaInk

    Text {
        anchors.centerIn: parent
        text: AuroraI18n.languageLabel + " / " + AuroraI18n.alternateLanguageLabel
        color: AuroraTokens.mangaInk
        font.pixelSize: 12
        font.weight: Font.DemiBold
    }

    TapHandler {
        onTapped: {
            AuroraI18n.toggleLanguage()
            root.toggled()
        }
    }
}
