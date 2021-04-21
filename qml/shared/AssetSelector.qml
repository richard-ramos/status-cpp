import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import "../imports"
import im.status.desktop 1.0

Item {
    id: root
    property var balances
    property var selectedAsset
    width: 86
    height: 24

    function resetInternal() {
        balances = undefined
        selectedAsset = undefined
    }

    onSelectedAssetChanged: {
        if (selectedAsset && selectedAsset.symbol) {
            iconImg.source = "../app/img/tokens/" + selectedAsset.symbol.toUpperCase() + ".png"
            selectedTextField.text = selectedAsset.symbol.toUpperCase()
        }
    }

    onBalancesChanged: {
        if(!balances) return;
        selectedAsset = {
            name: balances[0].name,
            symbol: balances[0].symbol.toUpperCase(),
            value: balances[0].balance,
            address: balances[0].address
        }
    }

    Select {
        id: select
        width: parent.width
        bgColor: Style.current.transparent
        bgColorHover: Style.current.secondaryHover
        model: balances
        caretRightMargin: 7
        select.radius: 6
        select.height: root.height
        menu.width: 343
        selectedItemView: Item {
            anchors.fill: parent
            SVGImage {
                id: iconImg
                anchors.left: parent.left
                anchors.leftMargin: 4
                sourceSize.height: 24
                sourceSize.width: 24
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }

            StyledText {
                id: selectedTextField
                anchors.left: iconImg.right
                anchors.leftMargin: 4
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 15
                height: 22
                verticalAlignment: Text.AlignVCenter
            }
        }

        menu.delegate: menuItem
    }

    Component {
        id: menuItem
        MenuItem {
            id: itemContainer
            property bool isFirstItem: index === 0
            property bool isLastItem: index === balances.length - 1

            width: parent.width
            height: 72
            SVGImage {
                id: iconImg
                anchors.left: parent.left
                anchors.leftMargin: Style.current.padding
                anchors.verticalCenter: parent.verticalCenter
                width: 40
                height: 40
                sourceSize.height: height
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                source: "../app/img/tokens/" + modelData.symbol.toUpperCase() + ".png"
            }
            Column {
                anchors.left: iconImg.right
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter

                StyledText {
                    text: modelData.symbol.toUpperCase()
                    font.pixelSize: 15
                    height: 22
                }

                StyledText {
                    text: modelData.symbol === "ETH" ? "Ethereum" : modelData.name
                    color: Style.current.secondaryText
                    font.pixelSize: 15
                    height: 22
                }
            }
            Column {
                anchors.right: parent.right
                anchors.rightMargin: Style.current.padding
                anchors.verticalCenter: parent.verticalCenter
                StyledText {
                    font.pixelSize: 15
                    height: 22
                    text: parseFloat(modelData.balance).toFixed(4) + " " + modelData.symbol.toUpperCase()
                }
                StyledText {
                    font.pixelSize: 15
                    anchors.right: parent.right
                    height: 22
                    text: {
                        const price = walletModel.prices[modelData.symbol];
                        if(price == undefined){
                            return "...";
                        }
                        return Utils.toLocaleString(parseFloat(modelData.balance) * price, appSettings.locale, {minimumFractionDigits: 2, maximumFractionDigits: 2}) + " " + StatusSettings.Currency.toUpperCase()
                    }
                    color: Style.current.secondaryText
                }
            }
            background: Rectangle {
                color: itemContainer.highlighted ? Style.current.backgroundHover : Style.current.background
                radius: Style.current.radius

                // cover bottom left/right corners with square corners
                Rectangle {
                    visible: !isLastItem
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.radius
                    color: parent.color
                }

                // cover top left/right corners with square corners
                Rectangle {
                    visible: !isFirstItem
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: parent.radius
                    color: parent.color
                }
            }
            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: itemContainer
                onClicked: {
                    root.selectedAsset = { address: modelData.address, name: modelData.name, value: modelData.balance, symbol: modelData.symbol }
                    select.menu.close()
                }
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
