import QtQuick 2.3
import "../../../../../shared"
import "../../../../../imports"
import im.status.desktop 1.0

Column {
    property string authorCurrentMsg: "authorCurrentMsg"
    property int verticalMargin: 50

    property string profileImage

    id: channelIdentifier
    spacing: Style.current.padding
    visible: authorCurrentMsg === ""
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.top: parent.top
    anchors.topMargin: this.visible ? verticalMargin : 0

    Rectangle {
        id: circleId
        anchors.horizontalCenter: parent.horizontalCenter
        width: 120
        height: 120
        radius: 120
        border.width: chat.chatType == ChatType.OneToOne ? 2 : 0
        border.color: Style.current.border
        color: {
            if (chat.chatType == ChatType.OneToOne) {
                return Style.current.transparent
            }
            return chat.color
        }

        RoundedImage {
            visible: chat.chatType == ChatType.OneToOne
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: 120
            height: 120
            source: chat.chatType == ChatType.OneToOne ? appMain.getProfileImage(chat.contact, true) : ""
            smooth: false
            antialiasing: true
        }

        StyledText {
            visible: chat.chatType != ChatType.OneToOne
            text: Utils.removeStatusEns((chat.name.charAt(0) === "#" ? chat.name.charAt(1) : chat.name.charAt(0)).toUpperCase())
            opacity: 0.7
            font.weight: Font.Bold
            font.pixelSize: 51
            color: Style.current.white
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    StyledText {
        id: channelName
        wrapMode: Text.Wrap
        text: {
            switch(chat.chatType) {
                case ChatType.Public: return "#" + chat.name;
                case ChatType.OneToOne: return Utils.getUsernameLabel(chat.contact)
                default: return chat.name
            }
        }
        font.weight: Font.Bold
        font.pixelSize: 22
        color: Style.current.textColor
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Item {
        id: channelDescription
        width: visible ? 330 : 0
        height: visible ? childrenRect.height : 0
        anchors.horizontalCenter: parent.horizontalCenter

        StyledText {
            id: descText
            wrapMode: Text.Wrap
            text: {
                switch(chat.chatType) {
                    //% "Welcome to the beginning of the <span style='color: %1'>%2</span> group!"
                    case Constants.chatTypePrivateGroupChat: return qsTrId("welcome-to-the-beginning-of-the--span-style--color---1---2--span--group-").arg(Style.current.textColor).arg(chatsModel.activeChannel.name);
                    //% "Any messages you send here are encrypted and can only be read by you and <span style='color: %1'>%2</span>"
                    case ChatType.OneToOne: return qsTrId("any-messages-you-send-here-are-encrypted-and-can-only-be-read-by-you-and--span-style--color---1---2--span-").arg(Style.current.textColor).arg(Utils.getUsernameLabel(chat.contact));
                    default: return "";
                }
            }
            font.pixelSize: 14
            color: Style.current.darkGrey
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
        }
    }

    Item {
        visible: chat.chatType == Constants.chatTypePrivateGroupChat && !chat.isMember
        anchors.horizontalCenter: parent.horizontalCenter
        width: joinChat.width
        height: visible ? 100 : 10
        id: joinOrDecline

        StyledText {
            id: joinChat
            //% "Join chat"
            text: qsTrId("join-chat")
            font.pixelSize: 20
            color: Style.current.blue
            anchors.horizontalCenter: parent.horizontalCenter

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: {
                    chatsModel.groups.join()
                    joinOrDecline.visible = false;
                }
            }
        }

        StyledText {
            //% "Decline invitation"
            text: qsTrId("group-chat-decline-invitation")
            font.pixelSize: 20
            color: Style.current.blue
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: joinChat.bottom
            anchors.topMargin: Style.current.padding
            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: {
                    chatsModel.leaveActiveChat()
                }
            }
        }
    }

    Item {
        id: spacer
        // TODO: visible: chatsModel.activeChannel.chatType === Constants.chatTypePrivateGroupChat && chatsModel.activeChannel.isMember
        width: parent.width
        height: Style.current.bigPadding

    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:480;width:640}
}
##^##*/
