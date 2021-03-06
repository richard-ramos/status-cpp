import QtQuick 2.13
import QtQuick.Controls 2.13
import "../../../../shared"
import "../../../../shared/status"
import "../../../../imports"
import "../components"
import im.status.desktop 1.0

Rectangle {
    property string chatId: ""
    property string name: "channelName"
    property string lastMessage: "My latest message\n with a return"
    property string timestamp: "1605212622434"
    property string unviewedMessagesCount: "2"
    property string identicon: ""
    property string chatColor: ""
    property var contact
    property bool hasMentions: false
    property int chatType: ChatType.Public
    property int realChatType: {
        if (chatType === Constants.chatTypeCommunity) {
            // TODO add a check for private community chats once it is created
            return ChatType.Public
        }
        return chatType
    }

    property string searchStr: ""
    property bool isCompact: appSettings.useCompactMode
    property int contentType: 1
    property bool muted: false
    property bool hovered: false
    property bool enableMouseArea: true

    property string profileImage: realChatType == ChatType.OneToOne ? appMain.getProfileImage(contact) || ""  : ""

    id: wrapper
    color: {
      if (ListView.isCurrentItem) {
        return Style.current.menuBackgroundActive
      }
      if (wrapper.hovered) {
        return Style.current.menuBackgroundHover
      }
      return Style.current.transparent
    }
    anchors.right: parent.right
    anchors.top: applicationWindow.top
    anchors.left: parent.left
    radius: 8
    // Hide the box if it is filtered out
    property bool isVisible: searchStr === "" || name.includes(searchStr)
    visible: isVisible ? true : false
    height: isVisible ? (!isCompact ? 64 : 40) : 0

    StatusIdenticon {
        id: contactImage
        height: !isCompact ? 40 : 28
        width: !isCompact ? 40 : 28
        chatColor: wrapper.chatColor
        chatId: wrapper.chatId
        chatName: wrapper.name
        chatType: wrapper.realChatType
        identicon: wrapper.profileImage || wrapper.identicon
        anchors.left: parent.left
        anchors.leftMargin: !isCompact ? Style.current.padding : Style.current.smallPadding
        anchors.verticalCenter: parent.verticalCenter
    }

    SVGImage {
        id: channelIcon
        width: 16
        height: 16
        fillMode: Image.PreserveAspectFit
        source: "../../../img/channel-icon-" + (wrapper.realChatType == ChatType.Public ? "public-chat.svg" : "group.svg")
        anchors.left: contactImage.right
        anchors.leftMargin: !isCompact ? Style.current.padding : Style.current.smallPadding
        anchors.top: !isCompact ? parent.top : undefined
        anchors.topMargin: !isCompact ? Style.current.smallPadding : 0
        anchors.verticalCenter: !isCompact ? undefined : parent.verticalCenter
        visible: wrapper.realChatType != ChatType.OneToOne
    }

    StyledText {
        id: contactInfo
        text: {
            switch(wrapper.chatType){
                case ChatType.OneToOne:
                    return Utils.getUsernameLabel(wrapper.contact);
                case ChatType.PrivateGroupChat:
                    return Emoji.parse(Utils.filterXSS(wrapper.name))
                default:
                    return "#" + Utils.filterXSS(wrapper.name);
            }   
        }
        anchors.right: contactTime.left
        anchors.rightMargin: Style.current.smallPadding
        elide: Text.ElideRight
        color: muted ? Style.current.secondaryText : Style.current.textColor
        font.weight: Font.Medium
        font.pixelSize: 15
        anchors.left: channelIcon.visible ? channelIcon.right : contactImage.right
        anchors.leftMargin: channelIcon.visible ? 2 :
                                                  (!isCompact ? Style.current.padding : Style.current.halfPadding)
        anchors.top: !isCompact ? parent.top : undefined
        anchors.topMargin: !isCompact ? Style.current.smallPadding : 0
        anchors.verticalCenter: !isCompact ? undefined : parent.verticalCenter
    }
    
    StyledText {
        id: lastChatMessage
        visible: !isCompact
        text: {
            switch(contentType){
                //% "Image"
                case ContentType.Image: return qsTrId("image");
                //% "Sticker"
                case ContentType.Sticker: return qsTrId("sticker");
                //% "No messages"
                default: 
                    return lastMessage ? Emoji.parse(Utils.filterXSS(replaceUsernamesOnMessageMentions(lastMessage))).replace(/\n|\r/g, ' ') : qsTrId("no-messages")
            }
        }
        textFormat: Text.RichText
        clip: true // This is needed because emojis don't ellide correctly
        anchors.right: contactNumberChatsCircle.left
        anchors.rightMargin: Style.current.smallPadding
        elide: Text.ElideRight
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Style.current.smallPadding
        font.pixelSize: 15
        anchors.left: contactImage.right
        anchors.leftMargin: Style.current.padding
        color: Style.current.secondaryText
    }

    StyledText {
        id: contactTime
        visible: !isCompact
        text: Utils.formatDateTime(wrapper.timestamp, appSettings.locale)
        anchors.right: parent.right
        anchors.rightMargin: Style.current.padding
        anchors.top: parent.top
        anchors.topMargin: Style.current.smallPadding
        font.pixelSize: 11
        color: Style.current.secondaryText
    }
    Rectangle {
        id: contactNumberChatsCircle
        width: 22
        height: 22
        radius: 50
        anchors.right: parent.right
        anchors.rightMargin: !isCompact ? Style.current.padding : Style.current.smallPadding
        anchors.bottom: !isCompact ? parent.bottom : undefined
        anchors.bottomMargin: !isCompact ? Style.current.smallPadding : 0
        anchors.verticalCenter: !isCompact ? undefined : parent.verticalCenter
        color: Style.current.blue
        visible: (unviewedMessagesCount > 0) || wrapper.hasMentions
        StyledText {
            id: contactNumberChats
            text: wrapper.hasMentions ? '@' : (wrapper.unviewedMessagesCount < 100 ? wrapper.unviewedMessagesCount : "99+")
            font.pixelSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: Style.current.white
        }
    }

    MouseArea {
        enabled: enableMouseArea
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
          wrapper.hovered = true
        }
        onExited: {
          wrapper.hovered = false
        }
        onClicked: {
            if (mouse.button & Qt.RightButton) {
                channelContextMenu.openMenu(index)//, muted, chatType, name, chatId, identicon)
                return;
            }
            chatGroupsListView.currentIndex = index
        }
    }

}

/*##^##
Designer {
    D{i:0;formeditorColor:"#ffffff";height:64;width:640}
}
##^##*/
