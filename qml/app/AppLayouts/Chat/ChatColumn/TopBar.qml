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
      property bool isGroupChatOrOneToOne: chat.chatType === ChatType.PrivateGroupChat || chat.chatType === ChatType.OneToOne
      anchors.left: parent.left
      anchors.leftMargin: this.isGroupChatOrOneToOne ? Style.current.padding : Style.current.padding + 4
      anchors.top: parent.top
      anchors.topMargin: 4
      sourceComponent:  this.isGroupChatOrOneToOne ? chatInfoButton : chatInfo
    }

    Component {
        id: chatInfoButton
        StatusChatInfoButton {
            chatId: chat.id
            chatColor: chat.color;
            chatName: chat.name
            chatType: chat.chatType
            identicon: chatsModel.activeChannel.identicon
            muted: chat.muted
            identiconSize: 36

            onClicked: {
                switch (chat.chatType) {
                    case ChatType.PrivateGroupChat: 
                        groupInfoPopup.open()
                        break;
                    case ChatType.OneToOne:
                        const profileImage = appMain.getProfileImage(chatsModel.activeChannel.id)
                        openProfilePopup(chatsModel.activeChannel.name, chatsModel.activeChannel.id, profileImage || chatsModel.activeChannel.identicon)
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
                var menu = chatContextMenu;
                if(chat.chatType === ChatType.PrivateGroupChat){
                    menu = groupContextMenu
                }

                if (menu.opened) {
                    return menu.close()
                }

                menu.popup(moreActionsBtn.x, moreActionsBtn.height)
               
            }
            cursorShape: Qt.PointingHandCursor
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            PopupMenu {
                id: chatContextMenu
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                subMenuIcons: [
                    {
                        source: Qt.resolvedUrl("../../../img/fetch.svg"),
                        width: 16,
                        height: 16
                    }
                ]
                Action {
                    icon.source: "../../../img/close.svg"
                    icon.width: chatTopBarContent.iconSize
                    icon.height: chatTopBarContent.iconSize
                    //% "Clear history"
                    text: qsTrId("clear-history")
                    onTriggered: chatsModel.clearChatHistory(chatsModel.activeChannel.id)
                }
                FetchMoreMessages {}
                Action {
                    icon.source: "../../../img/delete.svg"
                    icon.width: chatTopBarContent.iconSize
                    icon.height: chatTopBarContent.iconSize
                    icon.color: Style.current.red
                    //% "Delete Chat"
                    text: qsTrId("delete-chat")
                    onTriggered: {
                      //% "Delete Chat"
                      deleteChatConfirmationDialog.title = qsTrId("delete-chat")
                      //% "Delete Chat"
                      deleteChatConfirmationDialog.confirmButtonLabel = qsTrId("delete-chat")
                      //% "Are you sure you want to delete this chat?"
                      deleteChatConfirmationDialog.confirmationText = qsTrId("delete-chat-confirmation")
                      deleteChatConfirmationDialog.open()
                    }
                }
            }

            PopupMenu {
                id: groupContextMenu
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                Action {
                    icon.source: "../../../img/group_chat.svg"
                    icon.width: chatTopBarContent.iconSize
                    icon.height: chatTopBarContent.iconSize
                    //% "Group Information"
                    text: qsTrId("group-information")
                    onTriggered: groupInfoPopup.open()
                }
                Action {
                    icon.source: "../../../img/close.svg"
                    icon.width: chatTopBarContent.iconSize
                    icon.height: chatTopBarContent.iconSize
                    //% "Clear history"
                    text: qsTrId("clear-history")
                    onTriggered: chatsModel.clearChatHistory(chatsModel.activeChannel.id)
                }
                Action {
                    icon.source: "../../../img/leave_chat.svg"
                    icon.width: chatTopBarContent.iconSize
                    icon.height: chatTopBarContent.iconSize
                    //% "Leave Group"
                    text: qsTrId("leave-group")
                    onTriggered: {
                      //% "Leave group"
                      deleteChatConfirmationDialog.title = qsTrId("leave-group")
                      //% "Leave group"
                      deleteChatConfirmationDialog.confirmButtonLabel = qsTrId("leave-group")
                      //% "Are you sure you want to leave this chat?"
                      deleteChatConfirmationDialog.confirmationText = qsTrId("are-you-sure-you-want-to-leave-this-chat-")
                      deleteChatConfirmationDialog.open()
                    }
                }
            }

            GroupInfoPopup {
                id: groupInfoPopup
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
