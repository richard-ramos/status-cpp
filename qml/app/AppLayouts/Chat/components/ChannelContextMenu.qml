import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "./"
import "../../../../shared"
import "../../../../imports"
import im.status.desktop 1.0

PopupMenu {
    property int channelIndex
    property var contextChannel
    property var groupInfoPopup

    id: channelContextMenu
    width: 175
    subMenuIcons: [
        /* { */
        /*     source:  Qt.resolvedUrl("../../../img/bell.svg"), */
        /*     width: 16, */
        /*     height: 16 */
        /* }, */
        {
            source: Qt.resolvedUrl("../../../img/fetch.svg"),
            width: 16,
            height: 16
        }
    ]

    function openMenu(index) {
        if (index !== undefined && index > -1) {
            channelContextMenu.channelIndex = index;
            channelContextMenu.contextChannel = chatsModel.get(index);
        }
        channelContextMenu.popup()
    }

    Action {
        enabled: channelContextMenu.contextChannel.chatType != ChatType.Public
        text: {
            if (channelContextMenu.contextChannel.chatType == Constants.chatTypeOneToOne) {
                //% "View Profile"
                return qsTrId("view-profile")
            }
            if (channelContextMenu.contextChannel.chatType == Constants.chatTypePrivateGroupChat) {
                //% "View Group"
                return qsTrId("view-group")
            }
            //% "Share Chat"
            return qsTrId("share-chat")
        }
        icon.source: "../../../img/group.svg"
        icon.width: 16
        icon.height: 16
        onTriggered: {
            if (channelContextMenu.contextChannel.chatType === Constants.chatTypeOneToOne) {
                const userProfileImage = appMain.getProfileImage(channelContextMenu.contextChannel.id)
                return openProfilePopup(
                  channelContextMenu.contextChannel.name,
                  channelContextMenu.contextChannel.id,
                  userProfileImage || channelContextMenu.contextChannel.identicon
                )
            }
            if (channelContextMenu.contextChannel.chatType === Constants.chatTypePrivateGroupChat) {
                return groupInfoPopup.openMenu(channelContextMenu.contextChannel)
            }
        }
    }

    Separator {}

    Action {
        text: channelContextMenu.contextChannel.muted ?
                  //% "Unmute chat"
                  qsTrId("unmute-chat") :
                  //% "Mute chat"
                  qsTrId("mute-chat")
        icon.source: "../../../img/bell.svg"
        icon.width: 16
        icon.height: 16
        onTriggered: {
            if (chatsModel.channelIsMuted(channelContextMenu.channelIndex)) {
                chatsModel.unmuteChannel(channelContextMenu.channelIndex)
                return
            }
            chatsModel.muteChannel(channelContextMenu.channelIndex)
        }
    }

    Action {
        //% "Mark as Read"
        text: qsTrId("mark-as-read")
        icon.source: "../../../img/check-circle.svg"
        icon.width: 16
        icon.height: 16
        onTriggered:  chatsModel.markAllMessagesAsRead(channelContextMenu.channelIndex)
    }
    FetchMoreMessages {}
    Action {
        //% "Clear History"
        text: qsTrId("clear-history")
        icon.source: "../../../img/close.svg"
        icon.width: 16
        icon.height: 16
        onTriggered: chatsModel.deleteChatHistory(channelContextMenu.channelIndex)
    }

    Separator {}

    Action {
        text: {
            if (channelContextMenu.contextChannel.chatType === Constants.chatTypeOneToOne) {
                //% "Delete chat"
                return qsTrId("delete-chat")
            }
            if (channelContextMenu.contextChannel.chatType === Constants.chatTypePrivateGroupChat) {
                //% "Leave group"
                return qsTrId("leave-group")
            }
            //% "Leave chat"
            return qsTrId("leave-chat")
        }
        icon.source: {
            if (channelContextMenu.contextChannel.chatType === Constants.chatTypeOneToOne) {
                return "../../../img/delete.svg"
            }
            return "../../../img/leave_chat.svg"
        }
        icon.width: 16
        icon.height: 16
        onTriggered: chatsModel.remove(channelContextMenu.channelIndex)
    }
}

