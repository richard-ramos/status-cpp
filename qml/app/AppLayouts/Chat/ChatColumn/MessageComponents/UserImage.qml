import QtQuick 2.3
import "../../../../../shared"
import "../../../../../imports"

Loader {
    active: isMessage && authorCurrentMsg !== authorPrevMsg
    height: active ? item.height : 0

    sourceComponent: Component {
        Item {
            id: chatImage
            width: identiconImage.width
            height: identiconImage.height

            RoundedImage {
                id: identiconImage
                width: 36
                height: 36
                border.width: 1
                border.color: Style.current.border
                showLoadingIndicator: false
                source: {
                    // TODO show current user profile picture or identicon
                    return !isCurrentUser ? contact.image : profileModel.profile.identicon
                }
                smooth: false
                antialiasing: true

                MouseArea {
                    cursorShape: Qt.PointingHandCursor
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    onClicked: {
                        clickMessage(true)
                    }
                }
            }
        }
    }
}
