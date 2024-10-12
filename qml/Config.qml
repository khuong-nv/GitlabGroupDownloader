import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.12
import Qt.labs.settings 1.1
Item {

    signal back()
    Settings {
        fileName: ".gdm_config"
        property alias default_password_7z: password.text
    }

    Rectangle {
        width: parent.width
        height: parent.height

        Column{
            spacing: 20
            width: parent.width * 0.8
            anchors.centerIn: parent

            RowLayout {
                spacing: 20
                width: parent.width

                Label {
                    Layout.preferredWidth: parent.width * 0.3
                    text: "Default password: "
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignVCenter
                }

                TextField {
                    Layout.preferredWidth: parent.width * 0.7
                    id: password
                    placeholderText: "Default password for .7z file"
                    width: parent.width
                    selectByMouse: true
                    font.pixelSize: 16
                }
            }



            Button {
                text: "Back"
                font.pixelSize: 16
                icon.source: "qrc:/icons/icons8-back-50.png"
                onClicked: back()
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
