import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../shared"
import "../../../shared/status"
import "../../../imports"
import "./components"
import "./ChatColumn"
import "./ChatColumn/ChatComponents"
import "./data"
import "../Wallet"
import im.status.desktop 1.0

StackLayout {
    id: root
    property bool isActiveChat: false

    property var chat

    property string chatId: chat.chatId
    property bool profilePopupOpened: false
    property alias chatMessages: chatMessages
    property var mailserverRequestTimer: null

    property int chatGroupsListViewCount: 0
    
    property bool isReply: false
    property bool isImage: false

    property bool isExtendedInput: isReply || isImage

    property bool isConnected: false
    property string contactToRemove: ""

    property var onActivated: function () {
        chatInput.textInput.forceActiveFocus(Qt.MouseFocusReason)
    }

    Component.onCompleted: {
        chatInput.textInput.forceActiveFocus(Qt.MouseFocusReason)
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.minimumWidth: 300

    currentIndex: 0


    property var idMap: ({})
    property var suggestionsObj: ([])

    function addSuggestionFromMessageList(i){
        /* TODO:
        const contactAddr = chatsModel.messageList.getMessageData(i, "publicKey");
        if(idMap[contactAddr]) return;
        suggestionsObj.push({
                                alias: chatsModel.messageList.getMessageData(i, "alias"),
                                ensName: chatsModel.messageList.getMessageData(i, "ensName"),
                                address: contactAddr,
                                identicon: chatsModel.messageList.getMessageData(i, "identicon"),
                                localNickname: chatsModel.messageList.getMessageData(i, "localName")
                            })
        chatInput.suggestionsList.append(suggestionsObj[suggestionsObj.length - 1]);
        idMap[contactAddr] = true;
        */
    }

    function populateSuggestions() {
        /*chatInput.suggestionsList.clear()
        const len = chatsModel.suggestionList.rowCount()

        idMap = {}

        for (let i = 0; i < len; i++) {
            const contactAddr = chatsModel.suggestionList.rowData(i, "address");
            if(idMap[contactAddr]) continue;
            const contactIndex = profileModel.contacts.list.getContactIndexByPubkey(chatsModel.suggestionList.rowData(i, "address"));

            suggestionsObj.push({
                                    alias: chatsModel.suggestionList.rowData(i, "alias"),
                                    ensName: chatsModel.suggestionList.rowData(i, "ensName"),
                                    address: contactAddr,
                                    identicon: profileModel.contacts.list.rowData(contactIndex, "thumbnailImage"),
                                    localNickname: chatsModel.suggestionList.rowData(i, "localNickname")
                                })

            chatInput.suggestionsList.append(suggestionsObj[suggestionsObj.length - 1]);
            idMap[contactAddr] = true;
        }
        const len2 = chatsModel.messageList.rowCount();
        for (let f = 0; f < len2; f++) {
            addSuggestionFromMessageList(f);
        }*/
    }


    function showReplyArea() {
        isReply = true;
        isImage = false;

        const message = chat.messages.get(SelectedMessage.messageId);
        if(!message) return;
        
        chatInput.showReplyArea(message)
    }

    function requestAddressForTransaction(address, amount, tokenAddress, tokenDecimals = 18) {
        amount =  utilsModel.eth2Wei(amount.toString(), tokenDecimals)
        chatsModel.transactions.requestAddress(chatsModel.activeChannel.id,
                                               address,
                                               amount,
                                               tokenAddress)
        txModalLoader.close()
    }
    function requestTransaction(address, amount, tokenAddress, tokenDecimals = 18) {
        amount =  utilsModel.eth2Wei(amount.toString(), tokenDecimals)
        chatsModel.transactions.request(chatsModel.activeChannel.id,
                                        address,
                                        amount,
                                        tokenAddress)
        txModalLoader.close()
    }


    ColumnLayout {
        spacing: 0

        RowLayout {
            id: chatTopBar
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillWidth: true
            z: 60
            spacing: 0
            TopBar {
                id: topBar
                chat: root.chat
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            z: 60
            Rectangle {
                Component.onCompleted: {
                    isConnected = Status.IsOnline;
                    if(!isConnected){
                        connectedStatusRect.visible = true 
                    }
                }

                id: connectedStatusRect
                Layout.fillWidth: true
                height: 40;
                color: isConnected ? Style.current.green : Style.current.darkGrey
                visible: false
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: Style.current.white
                    id: connectedStatusLbl
                    text: isConnected ? 
                        //% "Connected"
                        qsTrId("connected") :
                        //% "Disconnected"
                        qsTrId("disconnected")
                }
            }

            Timer {
                id: timer
            }

            Connections {
                target: Status
                onOnlineStatusChanged: {
                    if (connected == isConnected) return;
                    isConnected = connected;
                    if(isConnected){
                        timer.setTimeout(function(){ 
                            connectedStatusRect.visible = false;
                        }, 5000);
                    } else {
                        connectedStatusRect.visible = true;
                    }
                }
            }
        }

        ProfilePopup {
            id: profilePopup
        }

        RowLayout {
            id: chatContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            spacing: 0
            ChatMessages {
                id: chatMessages
                messageList: chat.messages
                isActiveChat: root.isActiveChat
                chat: root.chat
            }
       }

        StatusImageModal {
            id: imagePopup
        }

        EmojiReactions {
            id: reactionModel
        }

        Connections {
            target: chatsModel
            onActiveChannelChanged: {
                chatInput.suggestions.hide();
                chatInput.textInput.forceActiveFocus(Qt.MouseFocusReason)
                populateSuggestions();
            }
            onMessagePushed: {
                addSuggestionFromMessageList(messageIndex);
            }
        }
   
        Loader {
            id: loadingMessagesIndicator
            active: false
            sourceComponent: loadingIndicator
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: Style.current.padding
            anchors.bottomMargin: Style.current.padding * 5
        }

        /** Begin: Extract to component **/

        Component {
            id: loadingIndicator
            LoadingAnimation {}
        }


        Connections {
            target: mailserverModel
            onMailserverRequestSent: {
                loadingMessagesIndicator.active = true;
                
                if(mailserverRequestTimer != null && mailserverRequestTimer.running){
                    mailserverRequestTimer.stop();
                }
                mailserverRequestTimer = timer.setTimeout(function(){ 
                    loadingMessagesIndicator.active = false;
                }, 5000);
            }
        }

        Rectangle {
            id: inputArea
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            height: chatInput.height
            Layout.preferredHeight: height
            color: "transparent"

            StatusChatInput {
                id: chatInput
                visible: {
                    if(chat.chatType == ChatType.PrivateGroupChat) {
                        // Is member?
                        return chat.chatMembers.filter(x => x.id === StatusSettings.PublicKey && x.joined == true).length > 0;
                    }

                    const community = chatsModel.activeCommunity
                    if (chatsModel.activeChannel.chatType !== Constants.chatTypePrivateGroupChat &&
                            (!community.active ||
                            community.access === Constants.communityChatPublicAccess ||
                            community.admin)) {
                        return true
                    }
                    return chatsModel.activeChannel.isMember
                }
                enabled: !chat.contact || !chat.contact.isBlocked
                chatInputPlaceholder: (!chatInput.enabled) ?
                        //% "This user has been blocked."
                        qsTrId("this-user-has-been-blocked-") :
                        //% "Type a message."
                        qsTrId("type-a-message-")
                anchors.bottom: parent.bottom
                chatType: chat.chatType
                onSendTransactionCommandButtonClicked: {
                    if (chatsModel.activeChannel.ensVerified) {
                        txModalLoader.sourceComponent = cmpSendTransactionWithEns
                    } else {
                        txModalLoader.sourceComponent = cmpSendTransactionNoEns
                    }
                    txModalLoader.item.open()
                }
                onReceiveTransactionCommandButtonClicked: {
                    txModalLoader.sourceComponent = cmpReceiveTransaction
                    txModalLoader.item.open()
                }
                onStickerSelected: {
                    chatsModel.get(index).sendSticker(packId, hashId)
                }
                onSendMessage: {
                    if (chatInput.fileUrls.length > 0){
                        chatsModel.get(index).sendImage(chatInput.fileUrls[0]);
                    }
                    var msg = StatusUtils.plainText(Emoji.deparse(chatInput.textInput.text))
                    if (msg.length > 0){
                        msg = chatInput.interpretMessage(msg)
                        chatsModel.get(index).sendMessage(msg, chatInput.isReply ? SelectedMessage.messageId : "", Utils.isOnlyEmoji(msg), false);
                        chatInput.textInput.textFormat = TextEdit.PlainText;
                        if(event) event.accepted = true
                        sendMessageSound.stop();
                        sendMessageSound.play();

                        Qt.callLater(function(){
                            chatInput.textInput.clear();
                            chatInput.textInput.textFormat = TextEdit.RichText;
                        });

                    }

                }
            }
        }
    }

    Loader {
        id: txModalLoader
        function close() {
            if (!this.item) {
                return
            }
            this.item.close()
            this.closed()
        }
        function closed() {
            this.sourceComponent = undefined
        }
    }
    Component {
        id: cmpSendTransactionNoEns
        ChatCommandModal {
            id: sendTransactionNoEns
            onClosed: {
                txModalLoader.closed()
            }
            sendChatCommand: chatColumnLayout.requestAddressForTransaction
            isRequested: false
            //% "Send"
            commandTitle: qsTrId("command-button-send")
            title: commandTitle
            //% "Request Address"
            finalButtonLabel: qsTrId("request-address")
            selectRecipient.selectedRecipient: {
                return {
                    address: Constants.zeroAddress, // Setting as zero address since we don't have the address yet
                    alias: chatsModel.activeChannel.alias,
                    identicon: chatsModel.activeChannel.identicon,
                    name: chatsModel.activeChannel.name,
                    type: RecipientSelector.Type.Contact
                }
            }
            selectRecipient.selectedType: RecipientSelector.Type.Contact
            selectRecipient.readOnly: true
        }
    }
    Component {
        id: cmpReceiveTransaction
        ChatCommandModal {
            id: receiveTransaction
            onClosed: {
                txModalLoader.closed()
            }
            sendChatCommand: chatColumnLayout.requestTransaction
            isRequested: true
            //% "Request"
            commandTitle: qsTrId("wallet-request")
            title: commandTitle
            //% "Request"
            finalButtonLabel: qsTrId("wallet-request")
            selectRecipient.selectedRecipient: {
                return {
                    address: Constants.zeroAddress, // Setting as zero address since we don't have the address yet
                    alias: chatsModel.activeChannel.alias,
                    identicon: chatsModel.activeChannel.identicon,
                    name: chatsModel.activeChannel.name,
                    type: RecipientSelector.Type.Contact
                }
            }
            selectRecipient.selectedType: RecipientSelector.Type.Contact
            selectRecipient.readOnly: true
        }
    }
    Component {
        id: cmpSendTransactionWithEns
        SendModal {
            id: sendTransactionWithEns
            onClosed: {
                txModalLoader.closed()
            }
            selectRecipient.readOnly: true
            selectRecipient.selectedRecipient: {
                return {
                    address: "",
                    alias: chatsModel.activeChannel.alias,
                    identicon: chatsModel.activeChannel.identicon,
                    name: chatsModel.activeChannel.name,
                    type: RecipientSelector.Type.Contact,
                    ensVerified: true
                }
            }
            selectRecipient.selectedType: RecipientSelector.Type.Contact
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#ffffff";height:770;width:800}
}
##^##*/
