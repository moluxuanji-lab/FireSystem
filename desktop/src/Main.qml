import QtQuick
import QtQuick.Layouts
import FluentUI 1.0

FluWindow {
    id: window
    title: Qt.application.name
    width: 960
    height: 640
    minimumWidth: 640
    minimumHeight: 420
    visible: true

    Component.onCompleted: {
        FluApp.init(window)
        FluTheme.darkMode = FluThemeType.System
        FluTheme.animationEnabled = true
    }

    appBar: FluAppBar {
        title: window.title
        showDark: true
        darkClickListener: () => {
            if (FluTheme.dark) {
                FluTheme.darkMode = FluThemeType.Light
            } else {
                FluTheme.darkMode = FluThemeType.Dark
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        FluText {
            Layout.alignment: Qt.AlignHCenter
            text: Qt.application.name + " v" + Qt.application.version
            font: FluTextStyle.Title
        }

        FluText {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("FluentUI 集成成功")
            font: FluTextStyle.Body
        }

        FluFilledButton {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("测试提示")
            onClicked: window.showSuccess(qsTr("FireSystem 运行正常"))
        }
    }
}
