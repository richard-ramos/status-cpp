import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../../imports"
import "../../../../../shared"
import "../../../../../shared/status"
import "../../../Chat/components"
import im.status.desktop 1.0

Rectangle {
    property string name: "Jotaro Kujo"
    property string contactId: "0x04d8c07dd137bd1b73a6f51df148b4f77ddaa11209d36e43d8344c0a7d6db1cad6085f27cfb75dd3ae21d86ceffebe4cf8a35b9ce8d26baa19dc264efe6d8f221b"
    property string identicon: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII="
    property string localNickname: "JoJo"
    property var profileClick: function() {}
    property bool isContact: true
    property bool isBlocked: false
    property string searchStr: ""
    signal blockContactActionTriggered(name: string, address: string)
    signal removeContactActionTriggered(address: string)
    property bool isHovered: false
    id: container

    visible: isContact && (searchStr == "" || name.includes(searchStr))
    height: visible ? 64 : 0
    anchors.right: parent.right
    anchors.left: parent.left
    anchors.leftMargin: -Style.current.padding
    anchors.rightMargin: -Style.current.padding
    border.width: 0
    radius: Style.current.radius
    color: isHovered ? Style.current.backgroundHover : Style.current.transparent

    Component {
        id: profilePopupComponent
        ProfilePopup {
        }
    }

    StatusImageIdenticon {
        id: accountImage
        anchors.left: parent.left
        anchors.leftMargin: Style.current.padding
        anchors.verticalCenter: parent.verticalCenter
        source: identicon
    }

    StyledText {
        id: usernameText
        text: name
        elide: Text.ElideRight
        anchors.right: parent.right
        anchors.rightMargin: Style.current.padding
        font.pixelSize: 17
        anchors.top: accountImage.top
        anchors.topMargin: Style.current.smallPadding
        anchors.left: accountImage.right
        anchors.leftMargin: Style.current.padding
    }

    MouseArea {
        cursorShape: Qt.PointingHandCursor
        anchors.fill: parent
        hoverEnabled: true
        onEntered: container.isHovered = true
        onExited: container.isHovered = false
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button === Qt.RightButton) {
                contactContextMenu.popup()
                return
            }
            changeAppSection(Constants.chat)
            chatsModel.join(ChatType.OneToOne, contactId);
        }
    }

    Rectangle {
        property int iconSize: 14
        property bool isHovered: false
        readonly property color hoveredBg: Utils.setColorAlpha(Style.current.black, 0.1)
        id: menuButton
        height: 32
        width: 32
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Style.current.padding
        radius: 8
        color: isHovered ? hoveredBg : Style.current.transparent

        SVGImage {
            source: "../../../../img/dots-icon.svg"
            width: 18
            height: 4
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
            id: mouseArea
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            hoverEnabled: true
            onExited: {
                container.isHovered = false
                menuButton.isHovered = false
            }
            onEntered: {
                container.isHovered = true
                menuButton.isHovered = true
            }
            onClicked: {
              contactContextMenu.popup()
            }

            PopupMenu {
                id: contactContextMenu
                hasArrow: false
                Action {
                    icon.source: "../../../../img/profileActive.svg"
                    icon.width: menuButton.iconSize
                    icon.height: menuButton.iconSize
                    //% "View Profile"
                    text: qsTrId("view-profile")
                    onTriggered: {
                        openPopup(profilePopupComponent, {
                            contact: contactsModel.get(contactId)
                        });
                    }
                    enabled: true
                }
                Action {
                    icon.source: "../../../../img/message.svg"
                    icon.width: menuButton.iconSize
                    icon.height: menuButton.iconSize
                    //% "Send message"
                    text: qsTrId("send-message")
                    onTriggered: {
                        changeAppSection(Constants.chat)
                        chatsModel.join(ChatType.OneToOne, contactId);
                    }
                    enabled: !container.isBlocked
                }
                Action {
                    icon.source: "../../../../img/block-icon.svg"
                    icon.width: menuButton.iconSize
                    icon.height: menuButton.iconSize
                    //% "Block User"
                    text: qsTrId("block-user")
                    enabled: !container.isBlocked
                    onTriggered: {
                      container.blockContactActionTriggered(name, contactId)
                    }
                }
                Action {
                    icon.source: "../../../../img/remove-contact.svg"
                    icon.width: menuButton.iconSize
                    icon.height: menuButton.iconSize
                    icon.color: Style.current.red
                    text: qsTrId("remove-contact")
                    enabled: container.isContact
                    onTriggered: {
                      container.removeContactActionTriggered(contactId)
                    }
                }
                Action {
                    icon.source: "../../../../img/block-icon.svg"
                    icon.width: menuButton.iconSize
                    icon.height: menuButton.iconSize
                    icon.color: Style.current.red
                    //% "Unblock User"
                    text: qsTrId("unblock-user")
                    enabled: container.isBlocked
                    onTriggered: {
                      contactsModel.get(contactId).toggleBlock();
                      contactContextMenu.close()
                    }
                }
            }
        }
    }
}
