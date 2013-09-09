import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {

    RectangularGlow {
        id: effect
        anchors.fill: rect
        glowRadius: rectMouseArea.pressed ? 18 : 10
        spread: 0.4
        color: "steelblue"
        cornerRadius: rect.radius + glowRadius
    }

    Rectangle {
        id: rect
        color: "black"
        anchors.centerIn: parent
        anchors.fill: parent
        anchors.margins: 20
        radius: 25

        MouseArea{
            id: rectMouseArea
            anchors.fill: parent
        }
    }
}
