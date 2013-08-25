import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0


ApplicationWindow{
    id: mainItem

    title: "Hive Client"

    width: 600
    height: 360

//    ColumnLayout {
//        id: mainLayout
//        anchors.fill: parent

//        GroupBox{
//            id: connectionBox
//            title: "Connection"
//            anchors.horizontalCenter: parent.horizontalCenter

//            RowLayout{
//                Button{
//                    id: connectionButton
//                    text: "Connect"
//                }
//                TextInput{
//                    id: ipText
//                    text: "10.10.10.40"
//                }
//                TextInput{
//                    id: portText
//                    text: "1234"
//                }
//            }
//        }
//    }




    StatusBar{
        id: statusBar
        width: parent.width
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

    }
}


