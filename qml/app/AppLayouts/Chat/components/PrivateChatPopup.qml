import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import "./"
import im.status.desktop 1.0

ModalPopup {
    property string validationError: ""

    property string pubKey : "";
    property string ensUsername : "";

    function validate() {
        if (!Utils.isChatKey(chatKey.text) && !Utils.isValidETHNamePrefix(chatKey.text)) {
            //% "Enter a valid chat key or ENS username"
            validationError = qsTrId("enter-a-valid-chat-key-or-ens-username");
            pubKey = ""
            ensUsername.text = "";
        } else if (StatusSettings.PublicKey === chatKey.text) {
            //% "Can't chat with yourself"
            validationError = qsTrId("can-t-chat-with-yourself");
        } else {
            validationError = ""
        }
        return validationError === ""
    }

    property var resolveENS: Backpressure.debounce(popup, 500, function (ensName){
        noContactsRect.visible = false
        searchResults.loading = true
        searchResults.showProfileNotFoundMessage = false
        EnsUtils.pubKey(ensName, resolvedPubKey => {
            if(chatKey.text == ""){
                ensUsername.text = "";
                pubKey = "";
            } else if(resolvedPubKey == ""){
                ensUsername.text = "";
                searchResults.pubKey = pubKey = "";
                searchResults.showProfileNotFoundMessage = true
            } else {
                if (StatusSettings.PublicKey === resolvedPubKey) {
                    //% "Can't chat with yourself"
                    popup.validationError = qsTrId("can-t-chat-with-yourself");
                } else {
                    searchResults.username = EnsUtils.formatUsername(chatKey.text);
                    let userAlias = StatusUtils.generateAlias(resolvedPubKey)
                    userAlias = userAlias.length > 20 ? userAlias.substring(0, 19) + "..." : userAlias
                    searchResults.userAlias =  userAlias + " • " + Utils.compactAddress(resolvedPubKey, 4)
                    searchResults.pubKey = pubKey = resolvedPubKey;
                    popup.ensUsername = ensName;
                    popup.pubKey = resolvedPubKey;
                }
                searchResults.showProfileNotFoundMessage = false
            }
            searchResults.loading = false;
            noContactsRect.visible = pubKey === ""  && ensUsername.text === "" && addedContacts.count  == 0 && !profileNotFoundMessage.visible
        })
    });

    function onKeyReleased(){
        searchResults.pubKey = ""
        if (!validate()) {
            searchResults.showProfileNotFoundMessage = false
            noContactsRect.visible = false
            return;
        }

        chatKey.text = chatKey.text.trim();
        
        if (Utils.isChatKey(chatKey.text)){
            pubKey = chatKey.text;
            const contact = contactsModel.get(pubKey);
            if (!contact || !contact.isAdded) {
                searchResults.username = StatusUtils.generateAlias(pubKey)
                searchResults.userAlias = Utils.compactAddress(pubKey, 4)
                searchResults.pubKey = pubKey
            }
            noContactsRect.visible = false
            return;
        }
        
        Qt.callLater(resolveENS, chatKey.text)
    }

    function validateAndJoin(pk, ensName) {
        if (!validate() || pk.trim() === "" || validationError !== "") return;
        doJoin(pk, ensName)
    }
    function doJoin(pk, ensName) {
        chatsModel.join(ChatType.OneToOne, pk, Utils.isChatKey(ensName) ? "" : ensName);
        popup.close();
    }

    id: popup
    //% "New chat"
    title: qsTrId("new-chat")

    onOpened: {
        chatKey.text = "";
        pubKey = "";
        ensUsername.text = "";
        chatKey.forceActiveFocus(Qt.MouseFocusReason)
        existingContacts.visible = addedContacts.count > 0
        noContactsRect.visible = !existingContacts.visible
    }

    Input {
        id: chatKey
        //% "Enter ENS username or chat key"
        placeholderText: qsTrId("enter-contact-code")
        Keys.onEnterPressed: validateAndJoin(popup.pubKey, chatKey.text)
        Keys.onReturnPressed: validateAndJoin(popup.pubKey, chatKey.text)
        Keys.onReleased: {
            onKeyReleased();
        }
        textField.anchors.rightMargin: clearBtn.width + Style.current.padding + 2

        StatusIconButton {
            id: clearBtn
            icon.name: "close-icon"
            type: "secondary"
            visible: chatKey.text !== ""
            icon.width: 14
            icon.height: 14
            width: 14
            height: 14
            anchors.right: parent.right
            anchors.rightMargin: Style.current.padding
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                chatKey.text = ""
                chatKey.forceActiveFocus(Qt.MouseFocusReason)
                searchResults.showProfileNotFoundMessage = false
                searchResults.pubKey = popup.pubKey = ""
                noContactsRect.visible = false
            }
        }
    }

    StyledText {
        id: validationErrorMessage
        text: popup.validationError
        visible: popup.validationError !== ""
        font.pixelSize: 13
        color: Style.current.danger
        anchors.top: chatKey.bottom
        anchors.topMargin: Style.current.smallPadding
        anchors.horizontalCenter: parent.horizontalCenter
    }

    PrivateChatPopupExistingContacts {
        id: existingContacts
        anchors.topMargin: this.height > 0 ? Style.current.xlPadding : 0
        anchors.top: chatKey.bottom
        model: addedContacts
        filterText: chatKey.text
        onContactClicked: function (contact) {
            chatsModel.join(ChatType.OneToOne, contact.contactId);
            popup.close();
        }
        expanded: !searchResults.loading && popup.pubKey === "" && !searchResults.showProfileNotFoundMessage
    }

    PrivateChatPopupSearchResults {
        id: searchResults
        anchors.top: existingContacts.visible ? existingContacts.bottom : chatKey.bottom
        anchors.topMargin: Style.current.padding
        hasExistingContacts: existingContacts.visible
        loading: false
        onResultClicked: validateAndJoin(popup.pubKey, chatKey.text)
        onAddToContactsButtonClicked: {
            chatsModel.join(ChatType.OneToOne, popup.pubKey, popup.ensUsername);
            popup.close();
        }
    }

    NoFriendsRectangle {
        id: noContactsRect
        anchors.top: chatKey.bottom
        anchors.topMargin: Style.current.xlPadding * 3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

}

/*##^##
Designer {
    D{i:0;height:300;width:300}
}
##^##*/
