import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import im.status.desktop 1.0

RowLayout {
    property string network: ""
    property string networkName: ""
    property string newNetwork: ""

    ConfirmationDialog {
        id: confirmDialog
        title: qsTr("Warning!")
        confirmationText: qsTr("The account will be logged out. When you unlock it again, the selected network will be used")
        onConfirmButtonClicked: {
            StatusSettings.CurrentNetwork = newNetwork;
            Status.closeSession();
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
