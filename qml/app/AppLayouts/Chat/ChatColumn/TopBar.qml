import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../shared"
import "../../../../shared/status"
import "../../../../imports"
import "../components"
import im.status.desktop 1.0

Rectangle {
    property var chat
    property int iconSize: 16
    id: chatTopBarContent
    color: Style.current.background
    height: 56
    Layout.fillWidth: true
    border.color: Style.current.border
    border.width: 1

    Loader {
      property bool isGroupChatOrOneToOne: chat.chatType == ChatType.PrivateGroupChat || chat.chatType == ChatType.OneToOne
      anchors.left: parent.left
      anchors.leftMargin: this.isGroupChatOrOneToOne ? Style.current.padding : Style.current.padding + 4
      anchors.top: parent.top
      anchors.topMargin: 4
      sourceComponent:  this.isGroupChatOrOneToOne ? chatInfoButton : chatInfo
    }

    Component {
        id: groupInfoPopupComponent
        GroupInfoPopup {
            id: groupInfoPopup
        }
    }

    Component {
        id: chatInfoButton
        StatusChatInfoButton {
            chatId: chat.chatId
            chatColor: chat.color
            chatName: chat.name
            contact: chat.contact
            chatType: chat.chatType
            identicon: chat.identicon
            muted: chat.muted
            identiconSize: 36

            onClicked: {
                switch (chat.chatType) {
                    case ChatType.PrivateGroupChat: 
                        openPopup(groupInfoPopupComponent, {channel: chatsModel.get(index)});
                        break;
                    case ChatType.OneToOne:
                        openProfilePopup(true, contactsModel.get_or_create(chat.chatId))
                        break;
                }
            }
        }
    }

    Component {
        id: chatInfo
        StatusChatInfo {
            identiconSize: 36
            chatName: chat.name
            chatType: chat.chatType
            chatColor: chat.color
            muted: chat.muted
        }
    }


    Rectangle {
        id: moreActionsBtnContainer
        width: 40
        height: 40
        radius: Style.current.radius
        color: Style.current.transparent
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Style.current.smallPadding

        StyledText {
            id: moreActionsBtn
            text: "..."
            font.letterSpacing: 0.5
            font.bold: true
            lineHeight: 1.4
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 25
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                parent.color = Style.current.border
            }
            onExited: {
                parent.color = Style.current.transparent
            }

            onClicked: {
                chatContextMenu.openMenu(index)
            }
            cursorShape: Qt.PointingHandCursor
            acceptedButtons: Qt.LeftButton | Qt.RightButton


            ChannelContextMenu {
                id: chatContextMenu
            }
        }
    }

    ConfirmationDialog {
        id: deleteChatConfirmationDialog
        btnType: "warn"
        onConfirmButtonClicked: {
            chatsModel.remove(index)
            deleteChatConfirmationDialog.close()
        }
    }
}
