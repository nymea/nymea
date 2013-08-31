import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow{
    id: mainWindow
    menuBar: MenuBar{
        id: mainWindowMenuBar
        Menu {
            title: "File"
            MenuItem {
                text: "Close"
                shortcut: "Ctrl+Q"
                onTriggered: mainWindow.close()
            }
        }
    }
    statusBar: StatusBar {
        id: statusBar
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }
    title: "Hive Client"
    width: 600
    height: 500


    GroupBox {
        id: connectionGroupBox
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 20
        title: "Connection"

        RowLayout{
            anchors.fill: parent
            Button{
                id: connectButton
                text: "Connect"
                onClicked: {
                    settings.setIPaddress(ipTextInput.text)
                    settings.setPort(portTextInput.text)
                    client.connectToHost(ipTextInput.text,portTextInput.text)
                }
            }
            Text {
                id: ipLable
                text: "IP:"
                horizontalAlignment: Text.AlignHCenter
            }
            TextField {
                id: ipTextInput
                text: "10.10.10.40"
                horizontalAlignment: TextInput.AlignHCenter
                font.pointSize: 9
            }
            Text {
                id: portLable
                text: "Port:"
                wrapMode: Text.NoWrap
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignHCenter
            }
            TextField {
                id: portTextInput
                text: "1234"
                horizontalAlignment: TextInput.AlignHCenter
                font.pointSize: 9
            }
        }
    }

    TabView{
        id: tabView
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: connectionGroupBox.bottom
        anchors.bottom: statusBar.top

        Tab{
            id: actorTab
            title: "Actor"

        }
        Tab{
            id: sensorTab
            title: "Sensor"
        }
        Tab{
            id: ruleTab
            title: "Rules"
        }
        Tab{
            id: settingsTab
            title: "Settings"
        }
    }
}



