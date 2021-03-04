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


    property var contact: null

    property bool isCurrentUser: contact && contact.id == StatusSettings.PublicKey

    readonly property int innerMargin: 20
    
    property bool noFooter: false

    signal blockButtonClicked(name: string, address: string)
    signal unblockButtonClicked(name: string, address: string)
    signal removeButtonClicked(address: string)

    signal contactUnblocked(publicKey: string)
    signal contactBlocked(publicKey: string)
    signal contactAdded(publicKey: string)
    signal contactRemoved(publicKey: string)

    clip: true
    noTopMargin: true

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
            source: getProfileImage(contact)
        }

        StyledTextEdit {
            id: profileName
            textFormat: Text.RichText
            text: Utils.getUsernameLabel(contact)
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
            textFormat: Text.RichText
            text: contact ? (((contact.ensVerified && contact.name !== "") || contact.localNickname) ? contact.alias : contact.id) : ""
            elide: contact && ((contact.ensVerified && contact.name !== "") || contact.localNickname)  ? Text.ElideNone : Text.ElideMiddle
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
                source: contact ? StatusUtils.generateQRCode(contact.id) : ""
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
            text: contact ? contact.name : ""
            anchors.top: parent.top
            visible: contact && contact.ensVerified
            height: visible ? implicitHeight : 0
            textToCopy: text
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
            text: contact ? contact.id : ""
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
            textToCopy: contact ? contact.id : ""
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
            //% "Share Profile URL"
            label: qsTrId("share-profile-url")
            text: "https://join.status.im/u/" + (contact ? (contact.id.substr(
                      0, 4) + "..." + contact.id.substr(contact.id.length - 5)) : "")
            anchors.top: separator.top
            anchors.topMargin: popup.innerMargin
            textToCopy: "https://join.status.im/u/" + (contact ? contact.id : "")
        }

        Separator {
            id: separator2
            visible: !isCurrentUser
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
            visible: !isCurrentUser
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
            visible: !isCurrentUser
            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Style.current.secondaryText
            }
        }

        StyledText {
            id: nicknameText
            textFormat: Text.RichText
            //% "None"
            text: contact && contact.localNickname ? Emoji.parse(Utils.filterXSS(contact.localNickname)) : qsTrId("none")
            anchors.right: nicknameCaret.left
            anchors.rightMargin: Style.current.padding
            anchors.verticalCenter: nicknameCaret.verticalCenter
            color: Style.current.secondaryText
            visible: !isCurrentUser
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
            type: "warn"
            showBorder: true
            bgColor: "transparent"
            borderColor: Style.current.border
            hoveredBorderColor: Style.current.transparent
            text: contact && contact.isBlocked ?
                      //% "Unblock User"
                      qsTrId("unblock-user") :
                      //% "Block User"
                      qsTrId("block-user")
            onClicked: {
                if (contact.isBlocked) {
                    unblockContactConfirmationDialog.open();
                    return;
                }
                blockContactConfirmationDialog.open();
            }
        }

        StatusButton {
            property bool isAdded: contact && contact.isAdded

            id: addToContactsButton
            anchors.right: sendMessageBtn.left
            anchors.rightMargin: sendMessageBtn.visible ? Style.current.padding : 0
            text: isAdded ?
                      //% "Remove Contact"
                      qsTrId("remove-contact") :
                      //% "Add to contacts"
                      qsTrId("add-to-contacts")
            anchors.bottom: parent.bottom
            type: isAdded ? "warn" : "primary"
            showBorder: isAdded
            borderColor: Style.current.border
            bgColor: isAdded ? "transparent" : Style.current.buttonBackgroundColor
            hoveredBorderColor: Style.current.transparent
            visible: contact && !contact.isBlocked
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
            visible: contact && !contact.isBlocked && chatLayoutContainer.activeChatId !== contact.id
            width: visible ? implicitWidth : 0
            onClicked: {
                if (tabBar.currentIndex !== 0)
                    tabBar.currentIndex = 0
                chatsModel.join(ChatType.OneToOne, contact.id, contact.name);
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
