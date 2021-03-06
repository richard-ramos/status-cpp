import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import im.status.desktop 1.0

Item {
    id: advancedContainer
    Layout.fillHeight: true
    Layout.fillWidth: true

    Column {
        id: generalColumn
        anchors.top: parent.top
        anchors.topMargin: topMargin
        width: contentMaxWidth
        anchors.horizontalCenter: parent.horizontalCenter

        StatusSettingsLineButton {
            //% "Network"
            text: qsTrId("network")
            currentValue:  Utils.getNetworkName(StatusSettings.CurrentNetwork)
            onClicked: openPopup(networksModalComponent)
        }

        StatusSettingsLineButton {
            //% "Fleet"
            text: qsTrId("fleet")
            currentValue: StatusSettings.Fleet
            onClicked: openPopup(fleetModalComponent)
        }

        StatusSettingsLineButton {
            text: qsTr("Minimize on close")
            isSwitch: true
            switchChecked: !appSettings.quitOnClose
            onClicked: function (checked) {
                appSettings.quitOnClose = !checked
            }
        }

        Item {
            id: spacer1
            height: Style.current.bigPadding
            width: parent.width
        }

        Separator {
            anchors.topMargin: Style.current.bigPadding
            anchors.left: parent.left
            anchors.leftMargin: -Style.current.padding
            anchors.right: parent.right
            anchors.rightMargin: -Style.current.padding
        }

        StatusSectionHeadline {
            //% "Experimental features"
            text: qsTrId("experimental-features")
            topPadding: Style.current.bigPadding
            bottomPadding: Style.current.padding
        }

        StatusSettingsLineButton {
            //% "Wallet"
            text: qsTrId("wallet")
            isSwitch: true
            switchChecked: appSettings.walletEnabled
            onClicked: function (checked) {
                appSettings.walletEnabled = checked
            }
        }

        StatusSettingsLineButton {
            //% "Dapp Browser"
            text: qsTrId("dapp-browser")
            isSwitch: true
            switchChecked: appSettings.browserEnabled
            onClicked: function (checked) {
                appSettings.browserEnabled = checked
            }
        }

        StatusSettingsLineButton {
            //% "Communities"
            text: qsTrId("communities")
            isSwitch: true
            switchChecked: appSettings.communitiesEnabled
            onClicked: function (checked) {
                appSettings.communitiesEnabled = checked
            }
        }

        StatusSettingsLineButton {
            //% "Node Management"
            text: qsTrId("node-management")
            isSwitch: true
            switchChecked: appSettings.nodeManagementEnabled
            onClicked: function (checked) {
                appSettings.nodeManagementEnabled = checked
            }
        }
    }

    Component {
        id: networksModalComponent
        NetworksModal {
        }
    }
    
    Component {
        id: fleetModalComponent
        FleetsModal {
        }
    }
}

/*##^##
Designer {
    D{i:0;height:400;width:700}
}
##^##*/
