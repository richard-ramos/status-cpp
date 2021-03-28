import QtQuick 2.13
import "../../../../imports"
import "../../../../shared"
import im.status.desktop 1.0

Rectangle {
    property string channel: "status"
    property var onJoin: function() {}

    border.width: 1
    radius: 8
    width: children[0].width + 10
    height: 32
    border.color: Style.current.border
    color: Style.current.transparent

    StyledText {
        id: suggestedChannelText
        text: "#" + channel
        font.weight: Font.Medium
        color: Style.current.blue;
        anchors.top: parent.top;
        anchors.topMargin: 5;
        anchors.left: parent.left;
        anchors.leftMargin: 5;
        horizontalAlignment: Text.AlignLeft;
        font.pixelSize: 15
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            chatsModel.join(ChatType.Public, channel);
            onJoin()
        }
        cursorShape: Qt.PointingHandCursor
    }
}
