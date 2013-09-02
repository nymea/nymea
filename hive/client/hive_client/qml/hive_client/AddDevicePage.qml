import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Component {

    Rectangle{
        anchors.fill: parent

        ColumnLayout{
            id: addDeviceColumn

            anchors.horizontalCenter: parent.horizontalCenter

            GroupBox{
                id: nameGroupBox
                title: "Name"

                Row{
                    spacing: 5
                    Text{
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Name:"
                    }

                    TextField{
                        id: nameText
                        anchors.verticalCenter: parent.verticalCenter
                        placeholderText: "Device Name"
                    }
                }

            }
            GroupBox{
                id: deviceTypeGroupBox
                title: "Device Type"

                ComboBox {
                    id: deviceComboBox
                    model: ["actor","sensor"]
                }
            }
            GroupBox{
                id: communicationTypeGroupBox
                title: "Communication Type"

                ComboBox {
                    id: communicationTypeComboBox
                    model: ["RC433","RC868","ZigBee"]
                }
            }
            GroupBox{
                id: linecodeGroupBox
                title: "Linecode"
                visible: communicationTypeComboBox.currentText == "RC868" || communicationTypeComboBox.currentText == "RC433" ? true : false

                ComboBox {
                    id: linecodeComboBox
                    model: ["Switch","Light","Unicode","Manchester","Differential Manchester"]
                }
            }
            GroupBox{
                id: switchGroupBox
                title: "Switchsettings"
                visible: (linecodeComboBox.currentText == "Switch") && (communicationTypeComboBox.currentText == "RC868" || communicationTypeComboBox.currentText == "RC433") ? true : false

                RowLayout{
                    GridLayout{
                        columns: 6
                        Label{
                            text: "1"
                        }
                        Label{
                            text: "2"
                        }
                        Label{
                            text: "3"
                        }
                        Label{
                            text: "4"
                        }
                        Label{
                            text: "5"
                        }
                        Label{
                            text: "6"
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
                        CheckBox{
                            id: channel6
                        }
                    }
                    ComboBox {
                        id: switchComboBox
                        model: ["A","B","C","D","E"]
                    }
                }
            }
        }
    }
}
