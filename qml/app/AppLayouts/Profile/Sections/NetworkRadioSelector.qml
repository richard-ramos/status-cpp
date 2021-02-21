import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import im.status.desktop 1.0

RowLayout {
    id: root
    property string network: ""
    property string networkName: ""
    property string newNetwork: ""
    property var parentPopup;

    ConfirmationDialog {
        id: confirmDialog
        //% "Warning!"
        title: qsTrId("close-app-title")
        //% "The account will be logged out. When you unlock it again, the selected network will be used"
        confirmationText: qsTrId("logout-app-content")
        onConfirmButtonClicked: {
            StatusSettings.CurrentNetwork = newNetwork;
            close();
            root.parentPopup.close();
            Status.closeSession();
        }
        onClosed: {
            StatusSettings.currentNetworkChanged()
        }
    }

    width: parent.width
    StyledText {
        text: networkName == "" ? Utils.getNetworkName(network) : networkName
        font.pixelSize: 15
    }
    StatusRadioButton {
        id: radioProd
        Layout.alignment: Qt.AlignRight
        ButtonGroup.group: networkSettings
        rightPadding: 0
        checked: StatusSettings.CurrentNetwork === network
        onClicked: {
            if (StatusSettings.CurrentNetwork === network) return;
            newNetwork = network;
            confirmDialog.open();
        }
    }
}
