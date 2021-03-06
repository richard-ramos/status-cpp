
import QtQuick 2.3
import "../../../../../shared"
import "../../../../../imports"

StyledText {
    property bool notSent: outgoingStatus === "not-sent"
    property bool isExpired: (outgoingStatus == "sending" && (Math.floor(timestamp) + 180000) < Date.now())

    id: retryLbl
    color: Style.current.red
    //% "Resend"
    text: qsTrId("resend-message")
    font.pixelSize: Style.current.tertiaryTextFontSize
    visible: isCurrentUser && (notSent || isExpired) 
    MouseArea {
        cursorShape: Qt.PointingHandCursor
        anchors.fill: parent
        onClicked: {
            chatsModel.resendMessage(chatId, messageId)
        }
    }
}
