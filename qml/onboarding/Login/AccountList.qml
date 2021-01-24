import QtQuick 2.13
import QtQuick.Controls 2.13
import "./samples/"
import "../../imports"
import im.status.desktop 1.0


ListView {
    property var accounts: AccountsData {}
    property var isSelected: function () {}
    property var onAccountSelect: function () {}

    id: addressesView
    anchors.fill: parent
    model: accounts
    focus: true
    spacing: Style.current.smallPadding

    delegate: AddressView {
        username: Status.generateAlias(model.publicKey)
        address: model.publicKey
        identicon: Status.generateIdenticon(model.publicKey)
        isSelected: function (index, address) {
            return addressesView.isSelected(index, address)
        }
        onAccountSelect: function (index) {
            addressesView.onAccountSelect(index)
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
