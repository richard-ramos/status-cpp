import QtQuick 2.3
import "../../../../../shared"
import "../../../../../imports"

Loader {
    active: isMessage
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
                    return !isCurrentUser ? getProfileImage(contact) : identityImage.defaultThumbnail
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
