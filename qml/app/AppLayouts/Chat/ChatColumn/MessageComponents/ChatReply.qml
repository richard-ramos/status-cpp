import QtQuick 2.14
import "../../../../../shared"
import "../../../../../imports"

Loader {
    property int textFieldWidth: item ? item.textField.width : 0
    property int textFieldImplicitWidth: 0
    property int authorWidth: item ? item.authorMetrics.width : 0

    property bool longReply: false
    property color elementsColor: isCurrentUser ? Style.current.chatReplyCurrentUser : Style.current.secondaryText
    property var container
    property int chatHorizontalPadding
    property var chat
    property var replyMessage: chat != undefined ? chat.messages.get(responseTo) : null

    id: root
    active: responseTo !== "" && replyMessage !== null

    sourceComponent: Component {
        Item {
            property alias textField: lblReplyMessage
            property alias authorMetrics: txtAuthorMetrics

            id: chatReply
            // childrenRect.height shows a binding loop for some reason, so we use heights instead
            height: lblReplyAuthor.height + ((replyMessage.contentType === Constants.imageType ? imgReplyImage.height : lblReplyMessage.height) + 5 + 8)

            TextMetrics {
                id: txtAuthorMetrics
                font: lblReplyAuthor.font
                text: lblReplyAuthor.text
            }

            StyledTextEdit {
                id: lblReplyAuthor
                text: "↳" + (replyMessage != null ? Utils.getUsernameLabel(replyMessage.contact, isCurrentUser) : "")
                color: root.elementsColor
                readOnly: true
                selectByMouse: true
                wrapMode: Text.Wrap
                anchors.left: parent.left
                anchors.right: parent.right
            }

            ChatImage {
                id: imgReplyImage
                visible: replyMessage != null && replyMessage.contentType == Constants.imageType
                imageWidth: 50
                imageSource: replyMessage.image
                anchors.top: lblReplyAuthor.bottom
                anchors.topMargin: 5
                anchors.left: parent.left
                chatHorizontalPadding: 0
                container: root.container
            }

            StyledTextEdit {
                id: lblReplyMessage
                visible: replyMessage != null && replyMessage.contentType != Constants.imageType
                Component.onCompleted: textFieldImplicitWidth = implicitWidth
                anchors.top: lblReplyAuthor.bottom
                anchors.topMargin: 5
                text: `<style type="text/css">`+
                        `a {`+
                            `color: ${isCurrentUser && !appSettings.useCompactMode ? Style.current.white : Style.current.textColor};`+
                        `}`+
                        `a.mention {`+
                            `color: ${isCurrentUser ? Style.current.cyan : Style.current.turquoise};`+
                        `}`+
                        `</style>`+
                    `</head>`+
                    `<body>`+
                        `${Emoji.parse(Utils.linkifyAndXSS(replaceUsernamesOnMessageMentions(replyMessage.text)), "26x26")}`+
                    `</body>`+
                `</html>`
                textFormat: Text.RichText
                color: root.elementsColor
                readOnly: true
                selectByMouse: true
                wrapMode: Text.Wrap
                font.pixelSize: Style.current.secondaryTextFontSize
                anchors.left: parent.left
                width: root.longReply ? parent.width : implicitWidth
                z: 51
            }

            Separator {
                anchors.top: replyMessage != null && replyMessage.contentType == Constants.imageType ? imgReplyImage.bottom : lblReplyMessage.bottom
                anchors.topMargin: replyMessage.contentType == Constants.imageType ? 15 : 8
                anchors.left: lblReplyMessage.left
                anchors.right: lblReplyMessage.right
                anchors.rightMargin: root.chatHorizontalPadding
                color: root.elementsColor
            }
        }
    }
}

