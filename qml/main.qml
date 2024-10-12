import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.12
import MyModules 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 480
    title: qsTr("Gitlab group downloader")

    Login {
        id: login
        visible: true
        anchors.centerIn: parent
        anchors.fill: parent

        onAccess: {
            GitlabRequest.requestGroups()
            login.visible = false
            home.visible = true
            home.username = "Hi <b>" + username + "</b>"
        }
    }

    Home {
        id: home
        visible: false
        anchors.centerIn: parent
        anchors.fill: parent

        onBack: {
            login.visible = true
            home.visible = false
        }

        onOpenSettings: {
            config.visible = true
            home.visible = false
        }
    }

    Config {
        id: config
        visible: false
        anchors.centerIn: parent
        anchors.fill: parent

        onBack: {
            home.visible = true
            config.visible = false
        }
    }
}
