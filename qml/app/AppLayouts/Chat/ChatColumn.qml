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

    property var chat  // TODO: should be the data type of model

    property alias chatMessages: chatMessages

    property int chatGroupsListViewCount: 0
    
    property bool isReply: false
    property bool isImage: false

    property bool isExtendedInput: isReply || isImage

    property bool isConnected: false
    property string contactToRemove: ""

    property var onActivated: function () {
        chatInput.textInput.forceActiveFocus(Qt.MouseFocusReason)
    }

    property bool isBlocked: profileModel.contacts.isContactBlocked(chatsModel.activeChannel.id)


    Component.onCompleted: {
        chatInput.textInput.forceActiveFocus(Qt.MouseFocusReason)
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.minimumWidth: 300

    currentIndex: 0 // TODO:  chatsModel.activeChannelIndex > -1 && chatGroupsListViewCount > 0 ? 0 : 1

    property var idMap: {}

    function addSuggestionFromMessageList(i){
        /* TODO:
        const contactAddr = chatsModel.messageList.getMessageData(i, "publicKey");
        if(idMap[contactAddr]) return;
        chatInput.suggestionsList.append({
            alias: chatsModel.messageList.getMessageData(i, "alias"),
            ensName: chatsModel.messageList.getMessageData(i, "ensName"),
            address: contactAddr,
            identicon: chatsModel.messageList.getMessageData(i, "identicon"),
            localNickname: chatsModel.messageList.getMessageData(i, "localName")
        });
        idMap[contactAddr] = true;
        */
    }

    function populateSuggestions(){
        /*chatInput.suggestionsList.clear()
        const len = chatsModel.suggestionList.rowCount()

        idMap = {}

        for (let i = 0; i < len; i++) {
            const contactAddr = chatsModel.suggestionList.rowData(i, "address");
            if(idMap[contactAddr]) continue;
            chatInput.suggestionsList.append({
                alias: chatsModel.suggestionList.rowData(i, "alias"),
                ensName: chatsModel.suggestionList.rowData(i, "ensName"),
                address: contactAddr,
                identicon: chatsModel.suggestionList.rowData(i, "identicon"),
                localNickname: chatsModel.suggestionList.rowData(i, "localNickname")
            });
            idMap[contactAddr] = true;
        }
        const len2 = chatsModel.messageList.rowCount();
        for (let i = 0; i < len2; i++) {
            addSuggestionFromMessageList(i);
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

    Connections {
        target: profileModel.contacts
        onContactListChanged: {
            isBlocked = profileModel.contacts.isContactBlocked(chatsModel.activeChannel.id);
        }
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
                    isConnected = chatsModel.isOnline
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
                target: chatsModel
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

        Rectangle {
            id: inputArea
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            height: chatInput.height
            Layout.preferredHeight: height
            color: "transparent"
            
            Connections {
                target: chatsModel
                onLoadingMessagesChanged:
                    if(value){
                        loadingMessagesIndicator.active = true
                    } else {
                         timer.setTimeout(function(){ 
                            loadingMessagesIndicator.active = false;
                        }, 5000);
                    }
            }

            Loader {
                id: loadingMessagesIndicator
                active: chatsModel.loadingMessages
                sourceComponent: loadingIndicator
                anchors.right: parent.right
                anchors.bottom: chatInput.top
                anchors.rightMargin: Style.current.padding
                anchors.bottomMargin: Style.current.padding
            }

            Component {
                id: loadingIndicator
                LoadingAnimation {}
            }

            StatusChatInput {
                id: chatInput
                visible: {
                    const community = chatsModel.activeCommunity
                    if (chatsModel.activeChannel.chatType !== Constants.chatTypePrivateGroupChat &&
                            (!community.active ||
                            community.access === Constants.communityChatPublicAccess ||
                            community.admin)) {
                        return true
                    }
                    return chatsModel.activeChannel.isMember
                }
                enabled: !isBlocked
                chatInputPlaceholder: isBlocked ?
                        qsTr("This user has been blocked.") :
                        qsTr("Type a message.")
                anchors.bottom: parent.bottom
                recentStickers: chatsModel.stickers.recent
                stickerPackList: chatsModel.stickers.stickerPacks
                chatType: chatsModel.activeChannel.chatType
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
                    chatsModel.stickers.send(hashId, packId)
                }
                onSendMessage: {
                    if (chatInput.fileUrls.length > 0){
                        chatsModel.sendImage(chatInput.fileUrls[0], false);
                    }
                    var msg = Status.plainText(Emoji.deparse(chatInput.textInput.text))
                    if (msg.length > 0){
                        msg = chatInput.interpretMessage(msg)
                        chatsModel.get(index).sendMessage(msg, chatInput.isReply ? SelectedMessage.messageId : "", Utils.isOnlyEmoji(msg), false);
                        chatInput.textInput.textFormat = TextEdit.PlainText;
                        if(event) event.accepted = true
                        chatInput.messageSound.stop()
                        Qt.callLater(chatInput.messageSound.play);

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
            onOpened: {
                walletModel.getGasPricePredictions()
            }
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
