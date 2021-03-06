import QtQuick.Layouts 1.14
import QtQuick 2.13
import QtMultimedia 5.13
import QtQuick.Controls 2.13
import Qt.labs.settings 1.0
import "../../../imports"
import "../../../shared"
import "./ChatColumn"
import "."
import "components"
import im.status.desktop 1.0

SplitView {
    id: chatView
    property bool isActive: false
    handle: SplitViewHandle {}

    property string activeChatId: contactColumnLoader.item.list.count > 0 ? chatMsgStackLayout.children[chatMsgStackLayout.currentIndex].item.chatId : ""

    property var onActivated: function () {
        chatMsgStackLayout.children[chatMsgStackLayout.currentIndex].item.onActivated();
    }

    function openPopup(popupComponent, params = {}) {
        const popup = popupComponent.createObject(chatView, params);
        popup.open()
        return popup
    }

    function getContactListObject(dataModel, sourceModel) {
        const nbContacts = sourceModel.count;
        const contacts = []
        let contact
        for (let i = 0; i < nbContacts; i++) {
            contact = {
                name: sourceModel.get(i).name,
                localNickname:  sourceModel.get(i).localNickname,
                pubKey: sourceModel.get(i).contactId,
                identicon: sourceModel.get(i).identicon,
                thumbnailImage: sourceModel.get(i).image,
                isUser: false,
                isContact: sourceModel.get(i).isAdded
            }
            contacts.push(contact)
            if (dataModel) {
                dataModel.append(contact);
            }
        }
        return contacts
    }

    Connections {
        target: appMain
        onSettingsLoaded: {
            // Add recent
            chatView.restoreState(appSettings.chatSplitView)
        }
    }
    Component.onDestruction: appSettings.chatSplitView = this.saveState()

    Loader {
        id: contactColumnLoader
        SplitView.preferredWidth: Style.current.leftTabPrefferedSize
        SplitView.minimumWidth: Style.current.leftTabMinimumWidth
        SplitView.maximumWidth: Style.current.leftTabMaximumWidth
     // TODO:   sourceComponent: appSettings.communitiesEnabled && chatsModel.activeCommunity.active ? communtiyColumnComponent : contactsColumnComponent

        sourceComponent:  contactsColumnComponent
    }

    Component {
        id: contactsColumnComponent
        ContactsColumn {
        }
    }

    Component {
        id: communtiyColumnComponent
        CommunityColumn {}
    }

    Audio {
        id: sendMessageSound
        audioRole: Audio.NotificationRole
        source: "../../../../sounds/send_message.wav"
        volume: appSettings.volume
        muted: !appSettings.notificationSoundsEnabled
    }

    StackLayout {
        id: chatMsgStackLayout
        currentIndex: contactColumnLoader.item.list.count <= 0 ? 0 : contactColumnLoader.item.list.currentIndex + 1
        onCurrentIndexChanged: {
            if(currentIndex > 0 && !this.children[currentIndex].active){
                this.children[currentIndex].active = true;
            }
        }
        EmptyChat {}
        Repeater {
            model: chatsModel
            Loader {
                active: false
                sourceComponent: ChatColumn {
                    id: chatColumn
                    chat: model
                    chatGroupsListViewCount: contactColumnLoader.item.chatGroupsListViewCount
                    isActiveChat: (index + 1) === chatMsgStackLayout.currentIndex && chatView.isActive
                }
            }
            
        }
    }

    Connections {
        target: chatsModel
        onJoined: {
            contactColumnLoader.item.list.currentIndex = index;
        }
        onLeft: {
            // Use `index` if you want to show a different channel
            contactColumnLoader.item.list.currentIndex = -1;
        }
    }
   

    function openProfilePopup(showFooter, contact, parentPopup){
        var popup = profilePopupComponent.createObject(chatView, {contact: contact, noFooter: !showFooter});
        if(parentPopup){
            popup.parentPopup = parentPopup;
        }
        if(popup.isCurrentUser){
            popup.contact = UserIdentity;
            popup.contact.image = identityImage.defaultThumbnail;
            popup.height = 310;
            popup.noFooter = true;
        }
        popup.open();
    }

    property Component profilePopupComponent: ProfilePopup {
        id: profilePopup
        height: 504
        onClosed: {
            if(profilePopup.parentPopup){
                profilePopup.parentPopup.close();
            }
            destroy()
        }
    }


    ConfirmationDialog {
        id: removeContactConfirmationDialog
        // % "Remove contact"
        title: qsTrId("remove-contact")
        //% "Are you sure you want to remove this contact?"
        confirmationText: qsTrId("are-you-sure-you-want-to-remove-this-contact-")
        onConfirmButtonClicked: {
            if (profileModel.contacts.isAdded(chatColumn.contactToRemove)) {
              profileModel.contacts.removeContact(chatColumn.contactToRemove)
            }
            removeContactConfirmationDialog.parentPopup.close();
            removeContactConfirmationDialog.close();
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#ffffff";formeditorZoom:1.25;height:770;width:1152}
}
##^##*/
