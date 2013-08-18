import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Rectangle {
    id: mainWindow
    //title: "Hive Client"
    width: 500
    height: 360
    color: "#776262"

    RowLayout{
        anchors.centerIn: parent
        Button{
            text: "Button 1"
        }
        Button{
            text: "Button 1"
        }
    }


}
