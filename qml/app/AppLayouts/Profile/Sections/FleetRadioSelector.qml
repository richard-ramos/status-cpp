import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import im.status.desktop 1.0

StatusRadioButtonRow {
    property string fleetName: ""
    property string newFleet: ""
    property var parentPopup;
    id: root
    text: fleetName
    buttonGroup: fleetSettings
    checked: StatusSettings.Fleet === fleetName
    onRadioCheckedChanged: {
        if (checked) {
            if (StatusSettings.Fleet === fleetName) return;
            newFleet = fleetName;
            confirmDialog.open();
        }
    }
    ConfirmationDialog {
        id: confirmDialog
        //% "Warning!"
        title: qsTrId("close-app-title")
        //% "Change fleet to %1"
        confirmationText: qsTrId("change-fleet-to--1").arg(newFleet)
        onConfirmButtonClicked: {
            StatusSettings.Fleet = newFleet;
            close();
            root.parentPopup.close();
            Status.closeSession();
        }
        onClosed: StatusSettings.fleetChanged()
    }
}

