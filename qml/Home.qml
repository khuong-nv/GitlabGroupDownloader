import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import MyModules 1.0

Item {

    signal back()
    signal openSettings()

    property alias username: username.text
    ColumnLayout {
        spacing: 10
        anchors.centerIn: parent
        width: parent.width
        height: parent.height

        GroupBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: true
            title: qsTr("Groups")
            font.pointSize: 20
            ListView {
                anchors.topMargin: 10// Adjust height
                anchors.fill: parent
                model: GitlabRequest.model
                delegate: groupGitlab
                clip: true
            }
        }

        Rectangle {
            id: status
            visible: false
            Layout.preferredHeight: 30
            Layout.fillWidth: true

            RowLayout {
                visible: true

                anchors.fill: parent
                spacing: 10

                Text {
                    id: myText
                    color: "green"
                    text: ""
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 10
                    font.pointSize: 16
                }

                ProgressBar {
                    id: progress
                    Layout.fillWidth: true
                    indeterminate: true
                    Layout.rightMargin: 10
                    Layout.alignment: Qt.AlignRight
                }

                Timer {
                    id: timer
                    interval: 5000
                    running: false
                    repeat: false
                    onTriggered: {
                        status.visible = false
                    }

                }
            }

            Connections {
                target: GitlabRequest
                onStartDownload: {
                    console.log(fileName)
                    status.visible = true
                    timer.running = false
                    myText.color = "gray"
                    myText.text = "Downloading " + fileName
                    progress.visible = true
                    timer.running = false
                    btnBack.enabled = false
                    btnRefresh.enabled = false
                    btnConfig.enabled = false
                }
                onDownloadSuccess: {
                    myText.color = "green"
                    myText.text = "Downloaded done"
                    progress.visible = false
                    timer.running = true
                    btnBack.enabled = true
                    btnRefresh.enabled = true
                    btnConfig.enabled = true
                }
            }
        }

        FileSaveDialog {
            id: saveFile
            title: "Save file"
            filename: "download.zip"
            nameFilters: ["Archive (*.7z)", "All files (*)"]

            property string gitlab_url: ""
            property int gitlab_group_id: 0
            property bool isDownload: false

            function saveWithDefaultName(url, id, name) {
                console.log("Download url: ", url)
                filename = name + ".7z"
                gitlab_url = url
                gitlab_group_id = id
                saveFile.open()
            }

            onAccepted: {
                console.log("File selected: " + saveFile.fileUrl)
                GitlabRequest.download(gitlab_url, gitlab_group_id, saveFile.fileUrl);
                isDownload = true
            }
            onRejected: {

            }
        }

        RowLayout {
            id: rowLayout
            Layout.preferredWidth: parent.width
            Layout.fillWidth: false


            Button {
                x: 0
                id: btnBack
                text: qsTr("Back")
                font.pointSize: 16
                Layout.margins: 5
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                icon.source: "qrc:/icons/icons8-back-50.png"
                onClicked: back()
            }

            Button {
                id: btnRefresh
                font.pointSize: 16
                Layout.margins: 5
                Layout.fillWidth: false
                text: qsTr("Refresh")
                Layout.fillHeight: false
                Layout.columnSpan: 1
                transformOrigin: Item.Center
                Layout.rowSpan: 1
                icon.source: "qrc:/icons/icons8-refresh-50.png"

                onClicked: {
                    GitlabRequest.requestGroups()
                }
            }

            Button {
                id: btnConfig
                font.pointSize: 16
                Layout.margins: 5
                Layout.fillWidth: false
                text: qsTr("Settings")
                Layout.fillHeight: false
                Layout.columnSpan: 1
                transformOrigin: Item.Center
                icon.source: "qrc:/icons/icons8-settings-50.png"

                onClicked: openSettings()
            }

            Rectangle {
                id: rectangle
                width: 200
                height: 0
                color: "#ffffff"
                Layout.fillWidth: true
                Layout.fillHeight: false
            }

            Text {
                id: username
                text: ""
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                font.pixelSize: 18
                Layout.rightMargin: 10
                color: "green"
            }


        }

    }

    Component {
        id: groupGitlab
        Rectangle {
            width: ListView.view.width
            height: 50
            color: index % 2 == 0 ? "lightgray" : "white"

            RowLayout {
                spacing: 50
                anchors.fill: parent

                Text {
                    text: fullpath
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 10
                    font.pointSize: 16
                }

                Button {
                    background: Rectangle {
                        color: parent.down ? "#bbbbbb" :
                                             (parent.hovered ? "gray" : (index % 2 == 0 ? "lightgray" : "white"))
                    }
                    Layout.alignment: Qt.AlignRight
                    icon.source: "qrc:/icons/download.png"
                    onClicked: {
                        saveFile.saveWithDefaultName(url, Id, name)
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
