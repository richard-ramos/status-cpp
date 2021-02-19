import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import im.status.desktop 1.0

RowLayout {
    property string fleetName: ""
    property string newFleet: ""

    ConfirmationDialog {
        id: confirmDialog
        title: qsTr("Warning!")
        confirmationText: qsTr("Change fleet to %1").arg(newFleet)
        onConfirmButtonClicked: {
            StatusSettings.Fleet = newFleet;
            Status.closeSession();
        }
    }


    width: parent.width
    StyledText {
        text: fleetName
        font.pixelSize: 15
    }
    StatusRadioButton {
        id: radioProd
        Layout.alignment: Qt.AlignRight
        ButtonGroup.group: fleetSettings
        rightPadding: 0
        checked: StatusSettings.Fleet === fleetName
        onClicked: {
            if (StatusSettings.Fleet === fleetName) return;
            newFleet = fleetName;
            confirmDialog.open();
        }
    }
}
