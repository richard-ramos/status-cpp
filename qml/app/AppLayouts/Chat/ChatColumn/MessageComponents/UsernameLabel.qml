import QtQuick 2.3
import "../../../../../shared"
import "../../../../../imports"

Item {
    id: root
    property bool isHovered: false
    height: childrenRect.height
    width: chatName.width + (ensOrAlias.visible ? ensOrAlias.width + ensOrAlias.anchors.leftMargin : 0)
    property alias label: chatName
    visible: isMessage && authorCurrentMsg != authorPrevMsg

    StyledTextEdit {
        id: chatName
        textFormat: Text.RichText
        text: Utils.getUsernameLabel(contact, isCurrentUser)
        color: text.startsWith("@") || isCurrentUser || (contact.ensVerified && contact.name !== "") || contact.localNickname !== "" ? Style.current.blue : Style.current.secondaryText
        font.weight: Font.Medium
        font.pixelSize: Style.current.secondaryTextFontSize
        font.underline: root.isHovered
        readOnly: true
        wrapMode: Text.WordWrap
        selectByMouse: true
        MouseArea {
            cursorShape: Qt.PointingHandCursor
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                root.isHovered = true
            }
            onExited: {
                root.isHovered = false
            }
            onClicked: {
                clickMessage(true)
            }
        }
    }

    StyledText {
        id: ensOrAlias
        visible: (contact.ensVerified && contact.name !== "") || contact.localNickname
        text: {
            if(contact.name !== ""){
                if(contact.localNickname){
                    return contact.localNickname;
                } else {
                    return contact.alias;
                }
            } else if(contact.localNickname) {
                return contact.alias;
            } else {
                return "";
            }
        }
        color: Style.current.secondaryText
        font.pixelSize: chatName.font.pixelSize
        anchors.left: chatName.right
        anchors.leftMargin: chatName.visible ? 4 : 0
    }
}
