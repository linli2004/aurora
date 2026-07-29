import QtQuick

Item {
    id: root

    property var node: ({})
    property real screenX: 0.0
    property real screenY: 0.0
    property real projectedDepth: 0.0
    property color colorSignature: AuroraTokens.coolAccent
    property real networkOpacity: 1.0
    property real travelProgress: 0.0
    property real arrivedPulse: 0.0

    readonly property bool currentNode: node && node.current === true
    readonly property bool targetNode: node && node.target === true
    readonly property bool virtualNode: node && node.virtual === true
    readonly property real depth: projectedDepth
    readonly property real targetGlow:
        targetNode ? Math.max(travelProgress, arrivedPulse) : 0.0
    readonly property real baseSize:
        currentNode ? 15 : targetNode ? 14 : 7 + Math.max(0, depth) * 5

    x: screenX - width / 2
    y: screenY - height / 2
    width: 132
    height: 54
    opacity: networkOpacity
             * (currentNode || targetNode ? 1.0 : 0.46 + Math.max(0, depth) * 0.26)
    z: Math.round(100 + depth * 40)

    Rectangle {
        id: halo

        anchors.centerIn: dot
        width: dot.width * (currentNode ? 3.4 : targetNode ? 2.8 + root.targetGlow * 2.2 : 2.2)
        height: width
        radius: width / 2
        color: Qt.rgba(root.colorSignature.r,
                       root.colorSignature.g,
                       root.colorSignature.b,
                       currentNode ? 0.20 : targetNode ? 0.11 + root.targetGlow * 0.16 : 0.05)
        opacity: currentNode || targetNode ? 1.0 : 0.58
    }

    Rectangle {
        id: dot

        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.44
        anchors.verticalCenter: parent.verticalCenter
        width: root.baseSize + root.targetGlow * 5
        height: width
        radius: width / 2
        color: currentNode || targetNode
               ? root.colorSignature
               : Qt.rgba(root.colorSignature.r,
                         root.colorSignature.g,
                         root.colorSignature.b,
                         virtualNode ? 0.46 : 0.52)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, currentNode || targetNode ? 0.72 : 0.34)
    }

    Text {
        anchors.left: dot.right
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.verticalCenter: dot.verticalCenter
        text: node && node.title ? node.title : ""
        visible: text.length > 0
        color: currentNode || targetNode
               ? AuroraTokens.mangaInk
               : Qt.rgba(AuroraTokens.mangaInk.r,
                         AuroraTokens.mangaInk.g,
                         AuroraTokens.mangaInk.b,
                         0.56)
        opacity: currentNode ? 1.0 : targetNode ? 0.62 + root.targetGlow * 0.38 : 0.62
        font.pixelSize: currentNode || targetNode ? 13 : 11
        font.weight: currentNode || targetNode ? Font.DemiBold : Font.Medium
        maximumLineCount: currentNode || targetNode ? 2 : 1
        wrapMode: Text.Wrap
        elide: Text.ElideRight
    }
}
