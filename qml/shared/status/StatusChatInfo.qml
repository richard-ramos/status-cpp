import QtQuick 2.13
import QtQuick.Controls 2.13
import QtGraphicalEffects 1.13
import "../../imports"
import "../../shared"
import "../../shared/status"
import im.status.desktop 1.0

Item {
    id: root

    property string chatId
    property string chatName
    property int chatType
    property string identicon
    property string chatColor
    property int identiconSize: 40
    property bool isCompact: false
    property bool muted: false
    property var contact

    property string profileImage: chatType == ChatType.OneToOne ? appMain.getProfileImage(contact, true) || ""  : ""

    height: 48
    width: nameAndInfo.width + chatIdenticon.width + Style.current.smallPadding

    StatusIdenticon {
        id: chatIdenticon
        chatType: root.chatType
        chatName: root.chatName
        chatColor: root.chatColor
        identicon: root.profileImage
        width: root.isCompact ? 20 : root.identiconSize
        height: root.isCompact ? 20 : root.identiconSize
        anchors.verticalCenter: parent.verticalCenter
    }

    Item {
        id: nameAndInfo
        height: chatName.height + chatInfo.height
        width: childrenRect.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: chatIdenticon.right
        anchors.leftMargin: Style.current.smallPadding

        StyledText {
            id: chatName
            text: {
                switch(root.chatType) {
                    case ChatType.Public: return "#" + root.chatName;
                    case ChatType.OneToOne: return Utils.getUsernameLabel(contact)
                    default: return Emoji.parse(Utils.filterXSS(root.chatName))
                }
            }

            font.weight: Font.Medium
            font.pixelSize: 15
        }

        SVGImage {
            property bool hovered: false

            id: bellImg
            visible: root.muted
            source: "../../app/img/bell-disabled.svg"
            anchors.verticalCenter: chatName.verticalCenter
            anchors.left: chatName.right
            anchors.leftMargin: 4
            width: 12.5
            height: 12.5

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: bellImg.hovered ? Style.current.textColor : Style.current.darkGrey
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onEntered: bellImg.hovered = true
                onExited: bellImg.hovered = false
                onClicked: {
                    chatsModel.unmuteCurrentChannel()
                }
            }
        }

        StyledText {
            id: chatInfo
            color: Style.current.darkGrey
            text: {
                switch(root.chatType){
                    //% "Public chat"
                    case ChatType.Public: return qsTrId("public-chat")
                    case ChatType.OneToOne: return (contact.isAdded ?
                    //% "Contact"
                    qsTrId("chat-is-a-contact") :
                    //% "Not a contact"
                    qsTrId("chat-is-not-a-contact"))
                    case ChatType.PrivateGroupChat: 
                        let cnt = chatsModel.activeChannel.members.rowCount();
                        //% "%1 members"
                        if(cnt > 1) return qsTrId("%1-members").arg(cnt);
                        //% "1 member"
                        return qsTrId("1-member");
                    default: return "...";
                }
            }
            font.pixelSize: 12
            anchors.top: chatName.bottom
        }
    }
}

