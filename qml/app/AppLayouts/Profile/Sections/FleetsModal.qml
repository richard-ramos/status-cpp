import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"

ModalPopup {
    id: popup
    //% "Fleet"
    title: qsTrId("fleet")

    property string newFleet: "";
    
    Column {
        id: column
        spacing: Style.current.padding
        width: parent.width

        ButtonGroup { id: fleetSettings }

        FleetRadioSelector {
            fleetName: Constants.eth_prod
            parentPopup: popup
        }

        FleetRadioSelector {
            fleetName: Constants.eth_staging
            parentPopup: popup
        }

        FleetRadioSelector {
            fleetName: Constants.eth_test
            parentPopup: popup
        }
    }
}
