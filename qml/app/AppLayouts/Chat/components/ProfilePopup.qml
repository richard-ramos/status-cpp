import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"
import "./"
import im.status.desktop 1.0

ModalPopup {
    id: popup

    property Popup parentPopup


    property var contact

    property var identicon: ""
    property var userName: ""
    property string nickname: ""
    property var fromAuthor: ""
    property var text: ""
    property var alias: ""

    readonly property int innerMargin: 20
    
    property bool isEnsVerified: false
    property bool noFooter: false
    property bool isBlocked: false

    signal blockButtonClicked(name: string, address: string)
    signal unblockButtonClicked(name: string, address: string)
    signal removeButtonClicked(address: string)

    signal contactUnblocked(publicKey: string)
    signal contactBlocked(publicKey: string)
    signal contactAdded(publicKey: string)
    signal contactRemoved(publicKey: string)

    clip: true
    noTopMargin: true

    function openPopup(showFooter, userNameParam, fromAuthorParam, identiconParam, textParam, nicknameParam) {
        userName = userNameParam || ""
        nickname = nicknameParam || ""
        fromAuthor = fromAuthorParam || ""
        identicon = identiconParam || ""
        text = textParam || ""
        isEnsVerified = chatsModel.isEnsVerified(this.fromAuthor)
        isBlocked = profileModel.contacts.isContactBlocked(this.fromAuthor);
        alias = chatsModel.alias(this.fromAuthor) || ""
        
        noFooter = !showFooter;
        popup.open()
    }

    header: Item {
        height: 78
        width: parent.width

        RoundedImage {
            id: profilePic
            width: 40
            height: 40
            border.color: Style.current.border
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter
            source: contact.image
        }

        StyledTextEdit {
            id: profileName
            text: Utils.getUsernameLabel(contact, false) // TODO: isCurrentUser?
            anchors.top: parent.top
            anchors.topMargin: Style.current.padding
            anchors.left: profilePic.right
            anchors.leftMargin: Style.current.halfPadding
            font.bold: true
            font.pixelSize: 17
            readOnly: true
            wrapMode: Text.WordWrap
        }

        StyledText {
            text: ((contact.ensVerified && contact.name !== "") || contact.localNickname) ? contact.alias : contact.id
            elide: ((contact.ensVerified && contact.name !== "") || contact.localNickname)  ? Text.ElideNone : Text.ElideMiddle
            anchors.left: profilePic.right
            anchors.leftMargin: Style.current.smallPadding
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Style.current.padding
            anchors.right: qrCodeButton.left
            anchors.rightMargin: Style.current.padding
            anchors.topMargin: 2
            font.pixelSize: 14
            color: Style.current.secondaryText
        }

        StatusIconButton {
            id: qrCodeButton
            icon.name: "qr-code-icon"
            anchors.verticalCenter: profileName.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 52
            iconColor: Style.current.textColor
            onClicked: qrCodePopup.open()
            width: 32
            height: 32
        }

        Separator {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: -Style.current.padding
            anchors.right: parent.right
            anchors.rightMargin: -Style.current.padding
        }

        ModalPopup {
            id: qrCodePopup
            width: 320
            height: 320
            Image {
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                source: Status.generateQRCode(contact.id)
                anchors.horizontalCenter: parent.horizontalCenter
                height: 212
                width: 212
                mipmap: true
                smooth: false
            }
        }
    }

    Item {
        anchors.fill: parent

        TextWithLabel {
            id: ensText
            //% "ENS username"
            label: qsTrId("ens-username")
            text: contact.name
            anchors.top: parent.top
            visible: contact.ensVerified
            height: visible ? implicitHeight : 0
            textToCopy: contact.name
        }

        StyledText {
            id: labelChatKey
            //% "Chat key"
            text: qsTrId("chat-key")
            font.pixelSize: 13
            font.weight: Font.Medium
            color: Style.current.secondaryText
            anchors.top: ensText.bottom
            anchors.topMargin: ensText.visible ? popup.innerMargin : 0
        }

        Address {
            id: valueChatKey
            text: contact.id
            width: 160
            maxWidth: parent.width - (3 * Style.current.smallPadding) - copyBtn.width
            color: Style.current.textColor
            font.pixelSize: 15
            anchors.top: labelChatKey.bottom
            anchors.topMargin: 4
        }

        CopyToClipBoardButton {
            id: copyBtn
            anchors.top: labelChatKey.bottom
            anchors.left: valueChatKey.right
            textToCopy: contact.id
        }

        Separator {
            id: separator
            anchors.top: valueChatKey.bottom
            anchors.topMargin: popup.innerMargin
            anchors.left: parent.left
            anchors.leftMargin: -Style.current.padding
            anchors.right: parent.right
            anchors.rightMargin: -Style.current.padding
        }

        TextWithLabel {
            id: valueShareURL
            label: qsTr("Share Profile URL")
            text: "https://join.status.im/u/" + contact.id.substr(
                      0, 4) + "..." + contact.id.substr(contact.id.length - 5)
            anchors.top: separator.top
            anchors.topMargin: popup.innerMargin
            textToCopy: "https://join.status.im/u/" + contact.id
        }

        Separator {
            id: separator2
            anchors.top: valueShareURL.bottom
            anchors.topMargin: popup.innerMargin
            anchors.left: parent.left
            anchors.leftMargin: -Style.current.padding
            anchors.right: parent.right
            anchors.rightMargin: -Style.current.padding
        }

        TextWithLabel {
            id: chatSettings
            //% "Chat settings"
            label: qsTrId("chat-settings")
            //% "Nickname"
            text: qsTrId("nickname")
            anchors.top: separator2.top
            anchors.topMargin: popup.innerMargin
        }

        SVGImage {
            id: nicknameCaret
            source: "../../../img/caret.svg"
            rotation: -90
            anchors.right: parent.right
            anchors.rightMargin: Style.current.padding
            anchors.bottom: chatSettings.bottom
            anchors.bottomMargin: 5
            width: 13
            fillMode: Image.PreserveAspectFit
            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Style.current.secondaryText
            }
        }

        StyledText {
            id: nicknameText
            //% "None"
            text: contact.localNickname ? contact.localNickname : qsTrId("none")
            anchors.right: nicknameCaret.left
            anchors.rightMargin: Style.current.padding
            anchors.verticalCenter: nicknameCaret.verticalCenter
            color: Style.current.secondaryText
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.left: chatSettings.left
            anchors.right: nicknameCaret.right
            anchors.top: chatSettings.top
            anchors.bottom: chatSettings.bottom
            onClicked: {
                nicknamePopup.open()
            }
        }

        NicknamePopup {
            id: nicknamePopup
            contact: popup.contact
        }
    }

    footer: Item {
        id: footerContainer
        visible: !noFooter
        width: parent.width
        height: children[0].height

        StatusButton {
            id: blockBtn
            anchors.right: addToContactsButton.left
            anchors.rightMargin: addToContactsButton ? Style.current.padding : 0
            anchors.bottom: parent.bottom
            type: "secondary"
            color: Style.current.red
            showBorder: true
            borderColor: Style.current.border
            text: contact.isBlocked ?
                      qsTr("Unblock User") :
                      qsTr("Block User")
            onClicked: {
                if (contact.isBlocked) {
                    unblockContactConfirmationDialog.open();
                    return;
                }
                blockContactConfirmationDialog.open();
            }
        }

        StatusButton {
            property bool isAdded: contact.isAdded

            id: addToContactsButton
            anchors.right: sendMessageBtn.left
            anchors.rightMargin: sendMessageBtn.visible ? Style.current.padding : 0
            text: isAdded ?
                      //% "Remove Contact"
                      qsTrId("remove-contact") :
                      //% "Add to contacts"
                      qsTrId("add-to-contacts")
            anchors.bottom: parent.bottom
            type: isAdded ? "secondary" : "primary"
            color: isAdded ? Style.current.danger : Style.current.primary
            showBorder: isAdded
            borderColor: Style.current.border
            visible: !isBlocked
            width: visible ? implicitWidth : 0
            onClicked: {
                if (contact.isAdded) {
                    removeContactConfirmationDialog.parentPopup = popup;
                    removeContactConfirmationDialog.open();
                } else {
                    contact.toggleAdd();
                    contactAdded(contact.id);
                    popup.close();
                }
            }
        }

        StatusButton {
            id: sendMessageBtn
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            //% "Send Message"
            text: qsTrId("send-message")
            visible: !contact.isBlocked && chatsModel.activeChannel.id !== contact.id
            width: visible ? implicitWidth : 0
            onClicked: {
                if (tabBar.currentIndex !== 0)
                    tabBar.currentIndex = 0
                chatsModel.joinChat(fromAuthor, Constants.chatTypeOneToOne)
                popup.close()
            }
        }

        BlockContactConfirmationDialog {
            id: blockContactConfirmationDialog
            onBlockButtonClicked: {
                contact.toggleBlock();
                blockContactConfirmationDialog.close();
                popup.close()
                contactBlocked(contact.id)
            }
        }

        UnblockContactConfirmationDialog {
            id: unblockContactConfirmationDialog
            onUnblockButtonClicked: {
                contact.toggleBlock();
                unblockContactConfirmationDialog.close();
                popup.close()
                contactUnblocked(contact.id)
            }
        }

        ConfirmationDialog {
            id: removeContactConfirmationDialog
            // % "Remove contact"
            title: qsTrId("remove-contact")
            //% "Are you sure you want to remove this contact?"
            confirmationText: qsTrId("are-you-sure-you-want-to-remove-this-contact-")
            onConfirmButtonClicked: {
                contact.toggleAdd();
                removeContactConfirmationDialog.close();
                popup.close();
                contactRemoved(contact.id);
            }
        }
    }
}
