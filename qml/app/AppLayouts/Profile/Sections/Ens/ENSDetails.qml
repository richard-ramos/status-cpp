import QtQuick 2.14
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14
import "../../../../../imports"
import "../../../../../shared"
import "../../../../../shared/status"
import im.status.desktop 1.0
Item {
    property string username: ""
    property string walletAddress: ""

    signal backBtnClicked();

    Component.onCompleted: {
        walletAddressLbl.visible = false;
        keyLbl.visible = false;

        EnsUtils.pubKey(username, function(pubkey){
            keyLbl.text = pubkey.substring(0, 20) + "..." + pubkey.substring(pubkey.length - 20);
            keyLbl.textToCopy = pubkey;
            keyLbl.visible = true;
        });

        EnsUtils.address(username, function(address){
            walletAddressLbl.text = address;
            walletAddressLbl.textToCopy = address;
            walletAddressLbl.visible = true;
        });
    }

    StyledText {
        id: sectionTitle
        text: username
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 24
        font.weight: Font.Bold
        font.pixelSize: 20
    }

    Component {
        id: loadingImageComponent
        LoadingImage {}
    }

    Loader {
        id: loadingImg
        active: !walletAddressLbl.visible || !keyLbl.visible
        sourceComponent: loadingImageComponent
        anchors.right: parent.right
        anchors.rightMargin: Style.current.padding
        anchors.top: parent.top
        anchors.topMargin: Style.currentPadding
    }

    TextWithLabel {
        id: walletAddressLbl
        //% "Wallet address"
        label: qsTrId("wallet-address")
        visible: false
        text:  ""
        textToCopy: ""
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: sectionTitle.bottom
        anchors.topMargin: 24
    }

    TextWithLabel {
        id: keyLbl
        visible: false
        //% "Key"
        label: qsTrId("key")
        text: ""
        textToCopy: ""
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: walletAddressLbl.bottom
        anchors.topMargin: 24
    }

    StatusButton {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Style.current.padding
        anchors.horizontalCenter: parent.horizontalCenter
        //% "Back"
        text: qsTrId("back")
        onClicked: backBtnClicked()
    }
}
