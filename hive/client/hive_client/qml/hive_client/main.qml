import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0


ApplicationWindow{
    id: window
    width: 600
    height: 360

    statusBar: StatusBar {
        RowLayout {
            Label { text: "Read Only" }
        }
    }
}
