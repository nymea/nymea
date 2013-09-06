import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Component {

    Rectangle{
        color: "black"
        Column{
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 30
            spacing: 20


            MainMenuButton{
                text: "Add device"
                onClicked: stackView.push(addDevicePage)
            }
            MainMenuButton{
                text: "Temperatur"
                onClicked: stackView.push(temperaturPage)
            }


        }


    }

}
