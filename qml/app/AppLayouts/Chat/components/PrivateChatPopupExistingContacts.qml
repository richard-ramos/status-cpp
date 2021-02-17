import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import "./"

Item {
    id: root
    property var model
    anchors.left: parent.left
    anchors.right: parent.right
    property string filterText: ""
    property bool expanded: true
    signal contactClicked(var contact)

    height: Math.min(contactListView.contentHeight, (expanded ? 320 : 192))
    ScrollView {
        anchors.fill: parent

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: contactListView.contentHeight > contactListView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

        ListView {
            anchors.fill: parent
            spacing: 0
            clip: true
            id: contactListView
            model: root.model
            delegate: Contact {
                showCheckbox: false
                contactId: model.contactId
                isContact: model.isAdded
                isUser: false
                name: Utils.getUsernameLabel(contactsModel.get(model.contactId))
                identicon: model.image
                visible: model.isAdded && (root.filterText === "" ||  // TODO: use SortFilterProxyModel instead
                    model.localNickname.toLowerCase().includes(root.filterText.toLowerCase()) || 
                    model.alias.toLowerCase().includes(root.filterText.toLowerCase()) || 
                    model.name.toLowerCase().includes(root.filterText.toLowerCase()) ||
                    model.contactId.toLowerCase().includes(root.filterText.toLowerCase()))
                onContactClicked: function () {
                    root.contactClicked(model)
                }
            }
        }
    }

}


