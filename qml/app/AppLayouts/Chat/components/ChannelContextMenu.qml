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

    Component {
        id: groupInfoPopupComponent
        GroupInfoPopup {
            id: groupInfoPopup
        }
    }

    Action {
        enabled: channelContextMenu.contextChannel && channelContextMenu.contextChannel.chatType != ChatType.Public
        text: {
            if(!channelContextMenu.contextChannel) return "";
            if (channelContextMenu.contextChannel.chatType == ChatType.OneToOne) {
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
            if (channelContextMenu.contextChannel.chatType == ChatType.OneToOne) {
                openProfilePopup(true, contactsModel.get_or_create(channelContextMenu.contextChannel.id))
            }
            if (channelContextMenu.contextChannel.chatType == Constants.chatTypePrivateGroupChat) {
                openPopup(groupInfoPopupComponent, {channel: chatsModel.get(index)});
            }
        }
    }

    Separator {}

    Action {
        text: channelContextMenu.contextChannel && channelContextMenu.contextChannel.muted ?
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
            if(!channelContextMenu.contextChannel) return "";
            if (channelContextMenu.contextChannel.chatType == ChatType.OneToOne) {
                //% "Delete chat"
                return qsTrId("delete-chat")
            }
            if (channelContextMenu.contextChannel.chatType == ChatType.PrivateGroupChat) {
                //% "Leave group"
                return qsTrId("leave-group")
            }
            //% "Leave chat"
            return qsTrId("leave-chat")
        }
        icon.source: {
            if(!channelContextMenu.contextChannel) return "";
            if (channelContextMenu.contextChannel.chatType == ChatType.OneToOne) {
                return "../../../img/delete.svg"
            }
            return "../../../img/leave_chat.svg"
        }
        icon.width: 16
        icon.height: 16
        onTriggered: chatsModel.remove(channelContextMenu.channelIndex)
    }
}

