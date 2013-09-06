import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Component {

    Rectangle{
        color: "black"
        ColumnLayout{
            id: addDeviceColumn
            anchors.left: parent.left
            anchors.leftMargin: 30
            anchors.right: parent.right
            anchors.rightMargin: 30
            GroupBox{
                id: nameGroupBox
                anchors.left: parent.left
                anchors.right: parent.right
                title: "Name"

                Row{
                    id: row1
                    spacing: 5

                    Label{
                        id: nameLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Name:"
                        color: "white"
                    }

                    TextField{
                        id: nameTextField
                        implicitWidth: 300
                        anchors.verticalCenter: parent.verticalCenter
                        placeholderText: "Device Name"
                    }
                }

            }
            GroupBox{
                id: deviceTypeGroupBox
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                title: "Device Type"

                ComboBox {
                    id: deviceComboBox
                    model: ["device","actor","sensor"]
                }
            }
            GroupBox{
                id: communicationTypeGroupBox
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                title: "Communication Type"

                ComboBox {
                    id: communicationTypeComboBox
                    model: ["RC433","RC868","ZigBee"]
                }
            }
            GroupBox{
                id: linecodeGroupBox
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                title: "Linecode"
                visible: communicationTypeComboBox.currentText == "RC868" || communicationTypeComboBox.currentText == "RC433" ? true : false

                ComboBox {
                    id: linecodeComboBox
                    model: ["Switch","Light","Unicode","Manchester","Differential Manchester"]
                }
            }
            GroupBox{
                id: switchGroupBox
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                title: "Switchsettings"
                visible: (linecodeComboBox.currentText == "Switch") && (communicationTypeComboBox.currentText == "RC868" || communicationTypeComboBox.currentText == "RC433") ? true : false

                RowLayout{
                    GridLayout{
                        columns: 5
                        Label{
                            text: "1"
                            color: "white"
                        }
                        Label{
                            text: "2"
                            color: "white"
                        }
                        Label{
                            text: "3"
                            color: "white"
                        }
                        Label{
                            text: "4"
                            color: "white"
                        }
                        Label{
                            text: "5"
                            color: "white"
                        }
                        CheckBox{
                            id: channel1
                        }
                        CheckBox{
                            id: channel2
                        }
                        CheckBox{
                            id: channel3
                        }
                        CheckBox{
                            id: channel4
                        }
                        CheckBox{
                            id: channel5
                        }
                    }
                    ComboBox {
                        id: switchComboBox
                        model: ["A","B","C","D","E"]
                    }
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
        Button{
            id: saveButton
            anchors.right: parent.right
            anchors.rightMargin: 30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10

            text: "Save"
            onClicked: {
                var deviceType = deviceComboBox.currentText
                var method = "add"
                // TODO: variant map...to send hive the correct add command with all parameters
                stackView.pop(this)
            }
        }
    }
}
