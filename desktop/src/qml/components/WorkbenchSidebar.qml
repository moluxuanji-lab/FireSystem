pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: sidebar
    required property var service
    required property bool compact
    property bool shortWindow: height < 735
    signal navigate(string page)
    readonly property var pages: service ? service.navigationPages : []
    function t(label) {
        return service ? service.i18n.t(label, service.i18n.revision) : label
    }
    gradient: Gradient {
        GradientStop { position: 0; color: WorkbenchStyle.rail }
        GradientStop { position: 1; color: Qt.tint(WorkbenchStyle.rail, Qt.alpha(WorkbenchStyle.panel, 0.35)) }
    }
    Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: WorkbenchStyle.line }

    component NavItem: WorkbenchNavButton {
        required property string pageId
        objectName: "nav-" + pageId
        width: parent ? parent.width : 0
        height: sidebar.shortWindow ? 33 : 37
        compact: sidebar.compact
        selected: sidebar.service && (sidebar.service.currentPage === pageId ||
            (pageId === "settings" && sidebar.service.currentPage.startsWith("settings-")))
        onClicked: sidebar.navigate(pageId)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: sidebar.compact ? 7 : 13
        anchors.rightMargin: anchors.leftMargin
        anchors.topMargin: sidebar.shortWindow ? 12 : 17
        spacing: 0

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: groups.height
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            Column {
                id: groups
                width: parent.width
                spacing: sidebar.shortWindow ? 12 : 19
                Repeater {
                    model: sidebar.service && sidebar.service.unlocked
                        ? sidebar.pages.filter(p => ["operations", "engineering", "maintenance"].includes(p.id))
                        : sidebar.service && sidebar.service.signedIn ? [] : [{id: "workstation", label: "工作站"}]
                    delegate: Column {
                        id: group
                        required property var modelData
                        width: groups.width
                        spacing: 3
                        AbstractButton {
                            id: sectionLink
                            visible: !sidebar.compact
                            width: parent.width
                            height: 22
                            leftPadding: 13
                            text: sidebar.t(group.modelData.label)
                            enabled: group.modelData.id !== "workstation"
                            onClicked: sidebar.navigate(group.modelData.id)
                            contentItem: Text {
                                text: sectionLink.text
                                color: sectionLink.visualFocus || sectionLink.hovered ? WorkbenchStyle.accent : WorkbenchStyle.muted
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }
                        Repeater {
                            model: sidebar.pages.filter(p => group.modelData.id === "workstation"
                                ? p.id === "login" : p.section === group.modelData.id && p.id !== "projects")
                            delegate: NavItem {
                                required property var modelData
                                pageId: modelData.id
                                glyph: modelData.glyph
                                text: sidebar.t(modelData.label)
                            }
                        }
                    }
                }
            }
        }
        Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: WorkbenchStyle.line }
        Column {
            Layout.topMargin: sidebar.shortWindow ? 5 : 8
            Layout.fillWidth: true
            spacing: 3
            NavItem { pageId: "settings"; glyph: "settings"; text: sidebar.t("设置") }
            NavItem { pageId: "help"; glyph: "help"; text: sidebar.t("帮助") }
        }
        Rectangle { Layout.fillWidth: true; Layout.margins: 4; Layout.topMargin: 9; implicitHeight: 1; color: WorkbenchStyle.line }
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: sidebar.compact ? 4 : 8
            Layout.bottomMargin: sidebar.shortWindow ? 9 : 15
            spacing: 10
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 29; implicitHeight: 29; radius: 7
                color: WorkbenchStyle.panel; border.color: WorkbenchStyle.line
                WorkbenchIcon { anchors.centerIn: parent; glyph: "layer"; size: 16 }
            }
            Column {
                visible: !sidebar.compact
                Layout.fillWidth: true
                Text { text: sidebar.t("本地仿真"); font.pixelSize: 12; font.weight: Font.DemiBold; color: WorkbenchStyle.ink }
                Text { text: "SIMULATION"; font.pixelSize: 9; font.letterSpacing: 1.5; color: WorkbenchStyle.muted }
            }
            Rectangle { visible: !sidebar.compact; implicitWidth: 6; implicitHeight: 6; radius: 3; color: WorkbenchStyle.good }
        }
    }
}
