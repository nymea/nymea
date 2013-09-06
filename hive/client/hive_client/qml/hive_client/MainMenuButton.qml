import QtQuick 2.0
import QtGraphicalEffects 1.0
Rectangle {
    id: buttonRectangle

    width: 300
    height: 200

    property string text
    signal clicked()

    color: "black"

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
        color: "#ed000000"
        anchors.centerIn: parent
        anchors.fill: parent
        anchors.margins: 20
        radius: 25
        border.width: 5
        border.color: "#000000"
        opacity: 1

        Text{
            id: buttonText
            anchors.centerIn: parent

            color: "#ffffff"
            styleColor: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 20
            font.family: "Arial"
            font.bold: true
            text: buttonRectangle.text
        }

        MouseArea{
            id: rectMouseArea
            anchors.fill: parent
            onClicked: {
                buttonRectangle.clicked()
            }
        }
    }

}




//import QtQuick 2.0

//Rectangle{
//    id: buttonRectangle

//    property string text
//    signal clicked()

//    color: buttonMousArea.pressed ? "lightsteelblue" : "steelblue"
//    border.width: buttonMousArea.pressed ? 4 : 2
//    border.color: "black"
//    radius: 10

//    Text{
//        id: buttonText
//        anchors.centerIn: parent
//        text: buttonRectangle.text
//    }
//    MouseArea{
//        id: buttonMousArea
//        anchors.fill: parent
//        onClicked: {
//            buttonRectangle.clicked()
//        }
//    }

//}


