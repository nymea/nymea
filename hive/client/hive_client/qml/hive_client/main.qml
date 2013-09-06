import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import hive 1.0

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
            MenuItem {
                text: "Disconnect"
                shortcut: "Ctrl+D"
                onTriggered: client.disconnectFromHost()
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
    minimumWidth: 800
    minimumHeight: 600


    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: connectionPage
    }

    Connections{
        target: client
        onConnectionChanged:{
            print("connection changed",client.isConnected)
            if(client.isConnected){
                stackView.push(mainPage)
            }else{
                stackView.clear()
                stackView.push(connectionPage)
            }
        }
    }

    Settings{
        id: settings
    }

    ConnectionPage{
        id: connectionPage
    }

    MainPage{
        id: mainPage
    }

    AddDevicePage{
        id: addDevicePage
    }
    TemperaturePage{
        id: temperaturPage
    }


}



