import QtQuick 2.13
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14
import "../../../imports"
import "../../../shared"
import im.status.desktop 1.0

Item {
    height: assetListView.height
    Component {
        id: assetViewDelegate

        Item {
            id: element
            anchors.right: parent.right
            anchors.left: parent.left
            height: 40

            Image {
                id: assetInfoImage
                width: 36
                height: 36
                source: modelData.symbol ? "../../img/tokens/" + modelData.symbol + ".png" : ""
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.verticalCenter: parent.verticalCenter
                onStatusChanged: {
                    if (assetInfoImage.status == Image.Error) {
                        assetInfoImage.source = "../../img/tokens/0-native.png"
                    }
                }
            }
            StyledText {
                id: assetSymbol
                text: modelData.symbol
                anchors.left: assetInfoImage.right
                anchors.leftMargin: Style.current.smallPadding
                anchors.top: assetInfoImage.top
                anchors.topMargin: 0
                font.pixelSize: 15
            }
            StyledText {
                id: assetFullTokenName
                text: modelData.symbol === "ETH" ? "Ethereum" : modelData.name
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.left: assetInfoImage.right
                anchors.leftMargin: Style.current.smallPadding
                color: Style.current.secondaryText
                font.pixelSize: 15
            }
            StyledText {
                id: assetValue
                text: modelData.balance + " " + modelData.symbol
                anchors.right: parent.right
                anchors.rightMargin: 0
                font.pixelSize: 15
                font.strikeout: false
            }
            StyledText {
                id: assetFiatValue
                color: Style.current.darkGrey
                text: {
                    const price = walletModel.prices[modelData.symbol];
                    if(price == undefined){
                        return "";
                    }
                    
                    return Utils.toLocaleString(parseFloat(modelData.balance) * price, appSettings.locale, {minimumFractionDigits: 2, maximumFractionDigits: 2}) + " " + StatusSettings.Currency.toUpperCase()
                }
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                font.pixelSize: 15
            }
        }
    }

    ListModel {
        id: assetModel

        ListElement {
            value: "123 USD"
            symbol: "ETH"
            fullTokenName: "Ethereum"
            fiatBalanceDisplay: "3423 ETH"
            image: "../../img/token-icons/eth.svg"
        }
    }

    ScrollView {
        anchors.fill: parent
        Layout.fillWidth: true
        Layout.fillHeight: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: assetListView.contentHeight > assetListView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

        ListView {
            id: assetListView
            spacing: Style.current.padding * 2
            anchors.fill: parent
            model: balances
            delegate: assetViewDelegate
            boundsBehavior: Flickable.StopAtBounds
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorColor:"#ffffff";height:480;width:640}
}
##^##*/
