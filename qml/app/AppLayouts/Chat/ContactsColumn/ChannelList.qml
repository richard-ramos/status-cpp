import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../shared"
import "../../../../imports"
import "../components"
import "./"

Item {
    property var channelModel
    property alias list: chatGroupsListView

    property alias channelListCount: chatGroupsListView.count
    property string searchStr: ""
    id: channelListContent
    width: parent.width
    height: childrenRect.height

    Timer {
        id: timer
    }

    ListView {
        Component.onCompleted : currentIndex = 0

        id: chatGroupsListView
        spacing: appSettings.useCompactMode ? 4 : Style.current.halfPadding
        anchors.top: parent.top
        height: childrenRect.height
        visible: height > (appSettings.useCompactMode ? 30 : 50)
        anchors.right: parent.right
        anchors.left: parent.left
        interactive: false
        model: channelListContent.channelModel
        currentIndex: 0
        delegate: Channel {
            name: model.name
            muted: model.muted
            lastMessage: model.lastMessage.text
            timestamp: model.timestamp
            chatType: model.chatType
            identicon: model.identicon
            unviewedMessagesCount: model.unviewedMessagesCount
            hasMentions: model.hasMentions
            contentType: model.contentType
            searchStr: channelListContent.searchStr
            chatColor: model.color
            chatId: model.chatId
        }
        onCountChanged: {
            if (count > 0 && chatsModel.activeChannelIndex > -1) {
                // If a chat is added or removed, we set the current index to the first value
                chatsModel.activeChannelIndex = 0;
                currentIndex = 0;
            } else {
                if (chatsModel.activeChannelIndex > -1) {
                    chatGroupsListView.currentIndex = 0;
                } else {
                    // Initial state. No chat has been selected yet
                    chatGroupsListView.currentIndex = -1;
                }
            }
        }
    }

    Rectangle {
        id: noSearchResults
        anchors.top: parent.top
        height: visible ? 300 : 0
        color: "transparent"
        visible: !chatGroupsListView.visible && channelListContent.searchStr !== ""
        anchors.left: parent.left
        anchors.right: parent.right

        StyledText {
            font.pixelSize: 15
            color: Style.current.darkGrey
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("No search results")
        }
    }

    GroupInfoPopup {
        id: groupInfoPopup
    }

    ChannelContextMenu {
        id: channelContextMenu
        groupInfoPopup: groupInfoPopup
    }

    Connections {
        target: chatsModel.chats
        onDataChanged: {
            // If the current active channel receives messages and changes its position,
            // refresh the currentIndex accordingly
            if(chatsModel.activeChannelIndex !== chatGroupsListView.currentIndex){
                chatGroupsListView.currentIndex = chatsModel.activeChannelIndex
            }
        }
    }

    Connections {
        target: chatsModel
        onActiveChannelChanged: {
            chatsModel.hideLoadingIndicator()
            chatGroupsListView.currentIndex = chatsModel.activeChannelIndex
            SelectedMessage.reset();
            chatColumn.isReply = false;
        }
    }

    Connections {
        target: chatsModel
        onAdded: {
            // TODO: determine if added due to joining a chat, or due to 1:1
            chatGroupsListView.currentIndex = index;
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
