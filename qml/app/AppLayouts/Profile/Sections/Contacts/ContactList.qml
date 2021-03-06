import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "./samples/"
import "../../../../../imports"
import "../../../../../shared"
import "../../../Chat/components"
import "."

ListView {
    id: contactList
    property var contacts: ContactsData {}
    property string searchStr: ""
    property string searchString: ""
    property string lowerCaseSearchString: searchString.toLowerCase()
    property string contactToRemove: ""

    width: parent.width

    model: contacts
    
    delegate: Contact {
        name: Utils.getUsernameLabel(contactsModel.get(model.contactId))
        contactId: model.contactId
        localNickname: model.localNickname
        identicon: model.image
        isContact: model.isAdded
        isBlocked: model.isBlocked
        visible: searchString === "" ||
                 model.alias.toLowerCase().includes(lowerCaseSearchString) ||
                 model.localNickname.toLowerCase().includes(lowerCaseSearchString) ||
                 model.name.toLowerCase().includes(lowerCaseSearchString) ||
                 model.contactId.toLowerCase().includes(lowerCaseSearchString)
        onBlockContactActionTriggered: {
            blockContactConfirmationDialog.contactName = name
            blockContactConfirmationDialog.contactAddress = address
            blockContactConfirmationDialog.open()
        }
        onRemoveContactActionTriggered: {
            removeContactConfirmationDialog.value = address
            removeContactConfirmationDialog.open()
        }
    }

    ProfilePopup {
      id: profilePopup
    }

    // TODO: Make BlockContactConfirmationDialog a dynamic component on a future refactor
    BlockContactConfirmationDialog {
        id: blockContactConfirmationDialog
        onBlockButtonClicked: {
            contactsModel.get(blockContactConfirmationDialog.contactAddress).toggleBlock();
            blockContactConfirmationDialog.close()
        }
    }

    // TODO: Make ConfirmationDialog a dynamic component on a future refactor
    ConfirmationDialog {
        id: removeContactConfirmationDialog
        title: qsTrId("remove-contact")
        //% "Are you sure you want to remove this contact?"
        confirmationText: qsTrId("are-you-sure-you-want-to-remove-this-contact-")
        onConfirmButtonClicked: {
            if(contactsModel.get(removeContactConfirmationDialog.value).isAdded){
                contactsModel.get(removeContactConfirmationDialog.value).toggleAdd();
            }
            removeContactConfirmationDialog.close()
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
