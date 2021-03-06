import QtQuick 2.13
import Qt.labs.platform 1.1
import QtQuick.Controls 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.13
import QtQml.Models 2.13
import QtGraphicalEffects 1.13
import QtQuick.Dialogs 1.3
import "../../../../shared"
import "../../../../imports"
import "../components"
import "./samples/"
import "./MessageComponents"
import SortFilterProxyModel 0.2
import im.status.desktop 1.0

ScrollView {
    id: root

    property alias chatLogView: chatLogView
    property var chat
    property bool isActiveChat: false
    property int unreadMessagesWhileInactive: 0
    property int lastVisibleIndex: -1
    onIsActiveChatChanged: {
        if(isActiveChat){
            // Restore scroll
            chatLogView.currentIndex = lastVisibleIndex + unreadMessagesWhileInactive;
            chatLogView.positionViewAtIndex(chatLogView.currentIndex, ListView.Visible)
            unreadMessagesWhileInactive = 0;
            lastVisibleIndex = -1;
        } else {
            var center_x = chatLogView.x + chatLogView.width / 2
            lastVisibleIndex = chatLogView.indexAt( center_x, chatLogView.y + chatLogView.contentY + chatLogView.height - 10)
        }
    }

    property var messageList: MessagesData {}
    property bool loadingMessages: false
    property real scrollY: chatLogView.visibleArea.yPosition * chatLogView.contentHeight
    property int newMessages: 0

    contentItem: chatLogView
    Layout.fillWidth: true
    Layout.fillHeight: true

    ScrollBar.vertical.policy: chatLogView.contentHeight > chatLogView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    ListView {
        property string currentNotificationChatId

        id: chatLogView
        anchors.fill: parent
        anchors.bottomMargin: Style.current.bigPadding
        spacing: appSettings.useCompactMode ? 0 : 4
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 10000
        Layout.fillWidth: true
        Layout.fillHeight: true
        verticalLayoutDirection: ListView.BottomToTop

        Timer {
            id: timer
        }

        Button {
            readonly property int buttonPadding: 5

            id: scrollDownButton
            visible: false
            height: 32
            width: nbMessages.width + arrowImage.width + 2 * Style.current.halfPadding + (nbMessages.visible ? scrollDownButton.buttonPadding : 0)
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: Style.current.padding
            background: Rectangle {
                color: Style.current.buttonSecondaryColor
                border.width: 0
                radius: 16
            }
            onClicked: {
                root.newMessages = 0
                scrollDownButton.visible = false
                chatLogView.scrollToBottom(true)
            }

            StyledText {
                id: nbMessages
                visible: root.newMessages > 0
                width: visible ? implicitWidth : 0
                text: root.newMessages
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                color: Style.current.pillButtonTextColor
                font.pixelSize: 15
                anchors.leftMargin: Style.current.halfPadding
            }

            SVGImage {
                id: arrowImage
                width: 24
                height: 24
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: nbMessages.right
                source: "../../../img/leave_chat.svg"
                anchors.leftMargin: nbMessages.visible ? scrollDownButton.buttonPadding : 0
                rotation: -90

                ColorOverlay {
                    anchors.fill: parent
                    source: parent
                    color: Style.current.pillButtonTextColor
                }
            }

            MouseArea {
               cursorShape: Qt.PointingHandCursor
               anchors.fill: parent
               onPressed: mouse.accepted = false
            }
        }

        function scrollToBottom(force, caller) {
            if (!force && !chatLogView.atYEnd) {
                // User has scrolled up, we don't want to scroll back
                return false
            }
            if (caller && caller !== chatLogView.itemAtIndex(chatLogView.count - 1)) {
                // If we have a caller, only accept its request if it's the last message
                return false
            }
            // Call this twice and with a timer since the first scroll to bottom might have happened before some stuff loads
            // meaning that the scroll will not actually be at the bottom on switch
            // Add a small delay because images, even though they say they say they are loaed, they aren't shown yet
            Qt.callLater(chatLogView.positionViewAtBeginning)
            timer.setTimeout(function() {
                 Qt.callLater(chatLogView.positionViewAtBeginning)
            }, 100);
            return true
        }

        function clickOnNotification(chatId) {
            applicationWindow.show()
            applicationWindow.raise()
            applicationWindow.requestActivate()
            chatsModel.setActiveChannel(chatId)
            appMain.changeAppSection(Constants.chat)
        }

        Connections {
            target: chatsModel.get(index)

            onSendingMessage: {
                chatLogView.scrollToBottom(true)
            }

            onSendingMessageFailed: {
                // TODO:
                sendingMsgFailedPopup.open();
            }
        }

        Connections {
            target: chat.messages
            onMessagesLoaded: {
                loadingMessages = false;
            }
            
            onNewMessagePushed: {
                if(!isActiveChat){
                    unreadMessagesWhileInactive++;
                }
                if (!chatLogView.scrollToBottom()) {
                    root.newMessages++
                }
            }

            onMessageNotificationPushed: function(chatId, msg, messageType, chatType, timestamp, identicon, username, hasMention, isAddedContact, channelName) {
                if (appSettings.notificationSetting == Constants.notifyAllMessages || 
                    (appSettings.notificationSetting == Constants.notifyJustMentions && hasMention)) {
                    if (chatType === Constants.chatTypeOneToOne && !appSettings.allowNotificationsFromNonContacts && !isAddedContact) {
                        return
                    }
                    if (chatId === chatsModel.activeChannel.id && applicationWindow.active === true) {
                        // Do not show the notif if we are in the channel already and the window is active and focused
                        return
                    }

                    chatLogView.currentNotificationChatId = chatId

                    let name;
                    if (appSettings.notificationMessagePreviewSetting === Constants.notificationPreviewAnonymous) {
                        name = "Status"
                    } else if (chatType === Constants.chatTypePublic) {
                        name = chatId
                    } else {
                        name = chatType === Constants.chatTypePrivateGroupChat ? Utils.filterXSS(channelName) : Utils.removeStatusEns(username)
                    }

                    let message;
                    if (appSettings.notificationMessagePreviewSetting > Constants.notificationPreviewNameOnly) {
                        switch(messageType){
                        //% "Image"
                        case Constants.imageType: message = qsTrId("image"); break
                        //% "Sticker"
                        case Constants.stickerType: message = qsTrId("sticker"); break
                        default: message = Emoji.parse(msg, "26x26").replace(/\n|\r/g, ' ')
                        }
                    } else {
                        //% "You have a new message"
                        message = qsTrId("you-have-a-new-message")
                    }

                    currentlyHasANotification = true
                    if (appSettings.useOSNotifications && systemTray.supportsMessages) {
                        systemTray.showMessage(name,
                                               message,
                                               SystemTrayIcon.NoIcon,
                                               Constants.notificationPopupTTL)
                    } else {
                        notificationWindow.notifyUser(chatId, name, message, chatType, identicon, chatLogView.clickOnNotification)
                    }
                }
            }
        }
/* TODO:
        Connections {
            target: chatsModel.communities

             onMembershipRequestChanged: function (communityName, accepted) {
                systemTray.showMessage("Status",
                                       accepted ? qsTr("You have been accepted into the ‘%1’ community").arg(communityName) :
                                                  qsTr("Your request to join the ‘%1’ community was declined").arg(communityName),
                                       SystemTrayIcon.NoIcon,
                                       Constants.notificationPopupTTL)
            }

            onMembershipRequestPushed: function (communityName, pubKey) {
                systemTray.showMessage(qsTr("New membership request"),
                                       qsTr("%1 asks to join ‘%2’").arg(Utils.getDisplayName(pubKey)).arg(communityName),
                                       SystemTrayIcon.NoIcon,
                                       Constants.notificationPopupTTL)
            }
        }
*/
        Connections {
            target: systemTray
            onMessageClicked: {
                chatLogView.clickOnNotification(chatLogView.currentNotificationChatId)
            }
        }

        property var loadMsgs : Backpressure.oneInTime(chatLogView, 500, function() {
            if(loadingMessages) return;
            loadingMessages = true;
            chatsModel.get(index).loadMoreMessages();
        });

        onContentYChanged: {
            scrollDownButton.visible = (contentHeight - (scrollY + height) > 400)
            if(scrollY < 500){
                loadMsgs();
            }
        }


        model: messageListModel
        delegate: Message {
            id: msgDelegate
            chat: root.chat
            contact: model.contact
            chatId: model.chatId
            message: model.parsedText
            plainText: model.plainText
            timestamp: parseInt(model.timestamp, 10)
            sticker: model.sticker
            contentType: model.contentType
            outgoingStatus: model.outgoingStatus
            responseTo: model.responseTo
            authorCurrentMsg: msgDelegate.ListView.section
            // The previous message is actually the nextSection since we reversed the list order
            authorPrevMsg: msgDelegate.ListView.nextSection
            imageClick: imagePopup.openPopup.bind(imagePopup)
            messageId: model.messageId
            emojiReactions: model.emojiReactions
            linkUrls: model.linkUrls
            communityId: model.communityId
            hasMention: model.hasMention
            prevMessageIndex: model.index === messageListModel.count - 1 ? -1 : model.index + 1
            scrollToBottom: chatLogView.scrollToBottom
        }
        section.property: "sectionIdentifier"
        section.criteria: ViewSection.FullString
    }

    SortFilterProxyModel {
        id: messageListModel
        sourceModel: messageList
        sorters: ExpressionSorter {
            expression: {
                return modelLeft.clock > modelRight.clock;
            }
        }
    }

    MessageDialog {
        id: sendingMsgFailedPopup
        standardButtons: StandardButton.Ok
        //% "Failed to send message."
        text: qsTrId("failed-to-send-message-")
        icon: StandardIcon.Critical
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
