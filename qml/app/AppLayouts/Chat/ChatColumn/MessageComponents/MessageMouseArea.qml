import QtQuick 2.13
import "../../../../../shared"
import "../../../../../imports"

MouseArea {
    enabled: !placeholderMessage
    cursorShape: chatText.hoveredLink ? Qt.PointingHandCursor : undefined
    acceptedButtons: Qt.RightButton | Qt.LeftButton
    z: 50
    onClicked: {
        if(mouse.button & Qt.RightButton) {
            messageContextMenu = clickMessage(false, isSticker, false);
            if (typeof isMessageActive !== "undefined") {
                isMessageActive = true
            }
            return;
        }
        if (mouse.button & Qt.LeftButton && isSticker /*&& stickersLoaded*/) { // TODO: open sticker market on click
            // openPopup(statusStickerPackClickPopup, {packId: stickerPackId} ) // TODO:
            return;
        }
    }
}
