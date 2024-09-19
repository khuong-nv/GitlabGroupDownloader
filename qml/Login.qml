import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

Item {
    visible: true

    signal access(string username)
    Rectangle {
        width: parent.width
        height: parent.height
        color: "white"

        Settings {
            fileName: ".gdm_config"
            property alias gitlab_home_url: gitlabUrl.text
            property alias private_key: privateTokenText.text
        }


        Column {
            spacing: 20
            width: parent.width * 0.8
            anchors.centerIn: parent

            Text {
                text: "GitLab Download Manager"
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextField {
                id: gitlabUrl
                placeholderText: "Gitlab URL"
                width: parent.width
                selectByMouse: true
                font.pixelSize: 16
            }


            TextField {
                id: privateTokenText
                placeholderText: "Personal access token"
                width: parent.width
                selectByMouse: true
                font.pixelSize: 16
            }

            Button {
                text: "Access"
                width: parent.width
                font.pixelSize: 16
                onClicked: {
                    GitlabRequest.setGitlabInfo(gitlabUrl.text, privateTokenText.text)
                }
            }

            // Error message display
            Text {
                id: errorMessage
                color: "red"
                visible: false
                text: "Invalid credentials. Please try again."
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    MessageDialog {
        id: messageDialog
        title: "Error"
        icon: StandardIcon.Critical
    }

    Connections {
        target: GitlabRequest
        onAccessSuccess: {
            access(username);
        }
        onAccessError: {
            messageDialog.text = "Can't access to gitlab with your url and personal private access. Please try again! \n\n" + error + ""
            messageDialog.visible = true
        }
    }
}
