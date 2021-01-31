import QtQuick 2.13
import "../../imports"
import "../../shared"
import "../../shared/status"
import im.status.desktop 1.0

Item {
    id: root

    property string chatName
    property int chatType
    property string identicon
    property string chatColor

    width: 40
    height: 40

    Loader {
        sourceComponent: root.chatType == ChatType.OneToOne || !!root.identicon ? imageIdenticon : letterIdenticon
        anchors.fill: parent
    }

    Component {
        id: letterIdenticon

        StatusLetterIdenticon {
            chatName: root.chatName
            chatColor: root.chatColor
            width: parent.width
            height: parent.height
        }
    }

    Component {
        id: imageIdenticon

        StatusImageIdenticon {
            source: root.identicon
            width: parent.width
            height: parent.height
        }
    }
}

