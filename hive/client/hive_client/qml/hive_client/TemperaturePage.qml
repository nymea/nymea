import QtQuick 2.0
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0

Component {

    Rectangle{
        color: "black"

        RectangularGlow {
            id: effect
            anchors.fill: rect
            glowRadius: 18
            spread: 0.4
            color: "steelblue"
            cornerRadius: rect.radius + glowRadius
        }

        Rectangle{
            id: rect
            anchors.fill: parent
            anchors.margins: 70
            color: "black"
            radius: 30
            Canvas {
                anchors.fill: parent
                contextType: "2d"

                Path{
                    id: myPath
                    startX: 0; startY: 100

                    PathCurve { x: 75; y: 75 }
                    PathCurve { x: 200; y: 150 }
                    PathCurve { x: 325; y: 25 }
                    PathCurve { x: 400; y: 100 }
                    PathCurve { x: 500; y: 100 }
                    PathCurve { x: 600; y: 100 }
                    PathCurve { x: 700; y: 300 }
                }
                onPaint: {
                    context.strokeStyle = Qt.rgba(.4,.6,.8);
                    context.path = myPath;
                    context.stroke();
                }
            }

        }

        Button{
            id: cancelButton
            anchors.left: parent.left
            anchors.leftMargin: 30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10

            text: "Cancel"
            onClicked: stackView.pop(this)

        }

    }
}
