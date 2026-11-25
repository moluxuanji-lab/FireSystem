pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import FluentUI

Rectangle {
    id: bar
    required property var service
    required property bool narrow
    signal toggleNavigation()
    signal navigate(string page)
    color: WorkbenchStyle.panel
    implicitHeight: narrow ? 51 : 57

    function t(label) {
        return service ? service.i18n.t(label, service.i18n.revision) : label
    }

    component HeaderButton: AbstractButton {
        id: button
        property string glyph: ""
        property bool showLabel: false
        property bool outlined: false
        property bool avatar: false
        hoverEnabled: true
        leftPadding: 6
        rightPadding: 6
        implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
        implicitHeight: 33
        ToolTip.visible: hovered && !showLabel
        ToolTip.text: text
        background: Rectangle {
            color: button.hovered ? WorkbenchStyle.hover : button.outlined ? WorkbenchStyle.surface : "transparent"
            border.color: button.visualFocus ? WorkbenchStyle.accent : button.outlined ? WorkbenchStyle.line : "transparent"
            radius: 5
        }
        contentItem: RowLayout {
            spacing: 8
            Rectangle {
                implicitHeight: button.avatar ? 27 : 18
                implicitWidth: implicitHeight
                radius: height / 2
                color: button.avatar ? WorkbenchStyle.tint : "transparent"
                border.color: button.avatar ? WorkbenchStyle.line : "transparent"
                WorkbenchIcon {
                    glyph: button.glyph
                    anchors.centerIn: parent
                    size: button.avatar ? 15 : 18
                    ink: button.avatar || button.outlined ? WorkbenchStyle.accent : WorkbenchStyle.muted
                }
            }
            Text {
                visible: button.showLabel
                text: button.text
                font.pixelSize: 13
                color: button.avatar ? WorkbenchStyle.accent : WorkbenchStyle.ink
            }
            WorkbenchIcon { visible: button.outlined && button.showLabel; glyph: "expand"; size: 13 }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: bar.narrow ? 10 : 22
        anchors.rightMargin: anchors.leftMargin
        spacing: bar.narrow ? 6 : 12

        HeaderButton { objectName: "toggleNavigationButton"; glyph: "menu"; text: bar.t("切换导航"); onClicked: bar.toggleNavigation() }
        AbstractButton {
            id: brand
            text: "FireSystem"
            implicitWidth: brandRow.implicitWidth
            implicitHeight: 40
            onClicked: if (bar.service) bar.navigate(bar.service.unlocked ? "overview" : bar.service.accessLanding())
            background: Rectangle { color: "transparent"; border.color: brand.visualFocus ? WorkbenchStyle.accent : "transparent"; radius: 5 }
            contentItem: RowLayout {
                id: brandRow
                spacing: 9
                Image { source: "firesystem.svg"; Layout.preferredWidth: bar.narrow ? 26 : 33; Layout.preferredHeight: 35; fillMode: Image.PreserveAspectFit }
                Text { text: "FireSystem"; color: WorkbenchStyle.ink; font.pixelSize: bar.narrow ? 18 : 20; font.weight: Font.DemiBold; font.letterSpacing: -0.65 }
            }
        }
        Rectangle { visible: !bar.narrow; implicitWidth: 1; implicitHeight: 18; color: WorkbenchStyle.line }
        Text {
            visible: !bar.narrow
            text: bar.t("轨道消防上位机")
            color: WorkbenchStyle.muted
            font.pixelSize: 13
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
        Item { visible: bar.narrow; Layout.fillWidth: true }
        HeaderButton {
            visible: bar.service && bar.service.unlocked
            glyph: "folder"; showLabel: bar.width > 900; outlined: true
            text: bar.t("环线基准项目")
            onClicked: bar.navigate("projects")
        }
        HeaderButton { glyph: "review"; showLabel: bar.width > 900; text: bar.t("评审记录"); onClicked: review.open() }
        HeaderButton { glyph: "search"; text: bar.t("页面搜索"); onClicked: search.open() }
        Rectangle { visible: !bar.narrow; implicitWidth: 1; implicitHeight: 18; color: WorkbenchStyle.line }
        HeaderButton {
            objectName: "headerLoginBtn"
            visible: !bar.service || !bar.service.signedIn
            glyph: "user"; showLabel: true; avatar: true
            text: bar.t("登录")
            onClicked: bar.navigate("login")
        }
        FluDropDownButton {
            id: profile
            objectName: "headerProfileDropdown"
            visible: bar.service && bar.service.signedIn
            readonly property string accountId: visible ? bar.service.currentAccountId : ""
            readonly property string role: visible ? bar.t(({admin: "管理员", debugger: "调试员", operator: "操作员"})[bar.service.currentAccountRole] || "") : ""
            text: accountId + " · " + role
            implicitWidth: Math.min(200, Math.max(120, profileRow.implicitWidth + 38))
            implicitHeight: 42
            leftPadding: 8
            rightPadding: 30
            background: Rectangle {
                color: profile.hovered ? WorkbenchStyle.hover : "transparent"
                border.color: profile.visualFocus ? WorkbenchStyle.accent : "transparent"
                radius: 5
            }
            contentItem: RowLayout {
                id: profileRow
                spacing: 8
                Rectangle {
                    implicitWidth: 30; implicitHeight: 30; radius: 15
                    color: WorkbenchStyle.tint; border.color: WorkbenchStyle.line
                    Text { anchors.centerIn: parent; text: profile.accountId.charAt(0).toUpperCase(); color: WorkbenchStyle.accent; font.bold: true }
                }
                ColumnLayout {
                    spacing: 0
                    Layout.fillWidth: true
                    Text { Layout.fillWidth: true; text: profile.accountId; color: WorkbenchStyle.ink; font.pixelSize: 13; font.bold: true; elide: Text.ElideRight }
                    Text { text: profile.role; color: WorkbenchStyle.muted; font.pixelSize: 11 }
                }
            }
            FluMenuItem { text: bar.t("切换账号"); onClicked: bar.navigate("login") }
            FluMenuItem { text: bar.t("账号设置"); onClicked: bar.navigate("settings-accounts") }
            FluMenuItem { text: bar.t("退出登录"); onClicked: bar.service.signOut() }
        }
    }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: WorkbenchStyle.line }

    component HeaderDialog: Dialog {
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(460, bar.width - 28)
        modal: true
        standardButtons: Dialog.Close
        palette.window: WorkbenchStyle.panel
        palette.windowText: WorkbenchStyle.ink
        palette.text: WorkbenchStyle.ink
        palette.buttonText: WorkbenchStyle.ink
        palette.button: WorkbenchStyle.surface
        background: Rectangle { radius: 10; color: WorkbenchStyle.panel; border.color: WorkbenchStyle.line }
    }
    HeaderDialog {
        id: review
        title: bar.t("页面评审")
        implicitHeight: 180
    }
    Shortcut { sequence: "Ctrl+K"; onActivated: search.open() }
    HeaderDialog {
        id: search
        title: bar.t("搜索页面")
        onOpened: { query.clear(); query.forceActiveFocus() }
        contentItem: ColumnLayout {
            FluTextBox { id: query; Layout.fillWidth: true; placeholderText: bar.t("输入页面名称") }
            ListView {
                id: results
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(350, contentHeight)
                clip: true
                model: {
                    if (!bar.service) return []
                    const unlocked = bar.service.unlocked
                    const term = query.text.trim().toLowerCase()
                    return bar.service.navigationPages.filter(p => (unlocked || bar.service.pageAvailable(p.id))
                        && String(p.id + p.label + bar.t(p.label)).toLowerCase().includes(term))
                }
                delegate: WorkbenchNavButton {
                    required property var modelData
                    width: results.width
                    text: bar.t(modelData.label)
                    glyph: modelData.glyph
                    onClicked: { search.close(); bar.navigate(modelData.id) }
                }
                ScrollBar.vertical: ScrollBar {}
            }
            Text { visible: results.count === 0; text: bar.t("未找到页面"); color: WorkbenchStyle.muted }
        }
    }
}
