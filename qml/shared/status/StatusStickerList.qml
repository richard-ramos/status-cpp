import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import "../../imports"
import "../../shared"
import im.status.desktop 1.0

GridView {
    property int packId;

    id: root
    visible: count > 0
    anchors.fill: parent
    cellWidth: 88
    cellHeight: 88
    focus: true
    clip: true
    signal stickerClicked(string hash, int packId)
    delegate: Item {
        width: stickerGrid.cellWidth
        height: stickerGrid.cellHeight
        Column {
            anchors.fill: parent
            anchors.topMargin: 4
            anchors.leftMargin: 4
            ImageLoader {
                width: 80
                height: 80
                source: "image://ipfs-cache/" + StatusUtils.decodeHash(modelData)
                onClicked: {
                    root.stickerClicked(modelData, packId)
                }
            }
        }
    }
}
