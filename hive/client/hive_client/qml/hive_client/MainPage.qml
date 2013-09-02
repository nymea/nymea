import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Component {

    Rectangle{
        anchors.fill: parent

        Column{
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 30

            Rectangle{
                id: addDeviceButton
                width: 150
                height: 70
                color: "#c8c8c8"
                border.color: "black"
                radius: 10
                Text{
                    anchors.centerIn: parent
                    text: "Add Device"
                }
                MouseArea{
                    id: addDeviceButtonMousArea
                    anchors.fill: parent
                    onPressed: addDeviceButton.color = "steelblue"
                    onReleased: addDeviceButton.color = "#c8c8c8"
                    onClicked: stackView.push(addDevicePage)
                }

            }

        }


    }


}

