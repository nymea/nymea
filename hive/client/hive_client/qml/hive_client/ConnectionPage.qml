import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Component {
    Rectangle{
        focus: connectButton.forceActiveFocus()
        color: "black"

        GroupBox {
            id: connectionGroupBox
            anchors.centerIn: parent

            title: "Connection"

            RowLayout{
                anchors.fill: parent
                Button{
                    id: connectButton
                    text: "Connect"
                    onClicked: {
                        settings.ipaddress = ipTextInput.text
                        settings.port = portTextInput.text
                        client.connectToHost(ipTextInput.text,portTextInput.text)
                    }
                }
                Text {
                    id: ipLable
                    text: "IP:"
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                }
                TextField {
                    id: ipTextInput
                    text: settings.ipaddress
                    horizontalAlignment: TextInput.AlignHCenter
                    font.pointSize: 9
                }
                Text {
                    id: portLable
                    text: "Port:"
                    color: "white"
                    wrapMode: Text.NoWrap
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignHCenter
                }
                TextField {
                    id: portTextInput
                    text: settings.port
                    horizontalAlignment: TextInput.AlignHCenter
                    font.pointSize: 9
                }
            }
        }

    }
}
