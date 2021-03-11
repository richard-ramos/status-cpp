import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import "../../imports"
import "../../shared"
import "../../shared/status"
import "../../app/AppLayouts/Chat/ChatColumn/samples"
import SortFilterProxyModel 0.2
import im.status.desktop 1.0

Popup {
    id: root
    property var recentStickers: StickerData {}
    property var stickerPackList: StickerPackData {}
    signal stickerSelected(string hashId, string packId)
    property int installedPacksCount: Object.keys(StatusSettings.InstalledStickerPacks).length
    property bool stickerPacksLoaded: stickerPackList.count > 0
    width: 360
    height: 440
    modal: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    background: Rectangle {
        radius: Style.current.radius
        color: Style.current.background
        border.color: Style.current.border
        layer.enabled: true
        layer.effect: DropShadow {
            verticalOffset: 3
            radius: 8
            samples: 15
            fast: true
            cached: true
            color: "#22000000"
        }
    }

    onOpened: {
        btnHistory.clicked();
    }

    onClosed: {
        stickerMarket.visible = false
        footerContent.visible = true
        stickersContainer.visible = true
    }

    Connections {
        target: Status
        onOnlineStatusChanged: {
            if(Status.isOnline){
                stickerPacksModel.reloadStickers();
            }
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        spacing: 0

        SortFilterProxyModel {
            id: stickerPackList
            sourceModel: stickerPacksModel
            // sorters: StringSorter { roleName: "name" }
        }

        StatusStickerMarket {
            id: stickerMarket
            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            stickerPacks: stickerPackList
            onInstallClicked: {
                stickerPacksModel.install(packId)
                stickerPackListView.itemAt(stickerPackListView.count - 1).clicked()
            }
            onUninstallClicked: {
                chatsModel.stickers.uninstall(packId)
             //   stickerGrid.model = recentStickers
                btnHistory.clicked()
            }
            onBackClicked: {
                stickerMarket.visible = false
                footerContent.visible = true
                stickersContainer.visible = true
            }
        }

        Item {
            id: stickersContainer
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            Layout.topMargin: 4
            Layout.bottomMargin: 0
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.preferredHeight: 400 - 4

            Item {
                id: noStickerPacks
                anchors.fill: parent
                visible: root.installedPacksCount === 0 || StatusSettings.RecentStickers.length === 0

                Image {
                    id: imgNoStickers
                    visible: lblNoStickersYet.visible || lblNoRecentStickers.visible
                    width: 56
                    height: 56
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 134
                    source: "../../app/img/stickers_sad_icon.svg"
                }

                Item {
                    id: noStickersContainer
                    width: parent.width
                    height: 22
                    anchors.top: imgNoStickers.bottom
                    anchors.topMargin: 8

                    StyledText {
                        id: lblNoStickersYet
                        visible: root.installedPacksCount === 0
                        anchors.fill: parent
                        font.pixelSize: 15
                        //% "You don't have any stickers yet"
                        text: qsTrId("you-don't-have-any-stickers-yet")
                        lineHeight: 22
                        horizontalAlignment: Text.AlignHCenter
                    }

                    StyledText {
                        id: lblNoRecentStickers
                        visible: stickerPackListView.selectedPackId === -1 && StatusSettings.RecentStickers.length === 0 && !lblNoStickersYet.visible
                        anchors.fill: parent
                        font.pixelSize: 15
                        //% "Recently used stickers will appear here"
                        text: qsTrId("recently-used-stickers")
                        lineHeight: 22
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                StatusButton {
                    visible: lblNoStickersYet.visible
                    //% "Get Stickers"
                    text: qsTrId("get-stickers")
                    anchors.top: noStickersContainer.bottom
                    anchors.topMargin: Style.current.padding
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        stickersContainer.visible = false
                        stickerMarket.visible = true
                        footerContent.visible = false
                    }
                }
            }
            StatusStickerList {
                id: stickerGrid
                packId: stickerPackListView.selectedPackId
                model: ListModel {

                } // recentStickers
                onStickerClicked: {
                    root.stickerSelected(hash, packId)
                    root.close()
                }
            }


            Component {
                id: loadingImageComponent
                LoadingImage {
                    width: 50
                    height: 50
                }
            }

            Loader {
                id: loadingGrid
                active: !stickerPacksLoaded && Status.IsOnline
                sourceComponent: loadingImageComponent
                anchors.centerIn: parent
            }
        }

        Item {
            id: footerContent
            Layout.leftMargin: 8
            Layout.fillWidth: true
            Layout.preferredHeight: 40 - 8 * 2
            Layout.topMargin: 8
            Layout.rightMargin: 8
            Layout.bottomMargin: 8
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

            StatusRoundButton {
                id: btnAddStickerPack
                size: "medium"
                icon.name: "plusSign"
                implicitWidth: 24
                implicitHeight: 24
                enabled: Status.IsOnline
                state: stickerPackList.count != 0 || !Status.IsOnline ? "default" : "pending"
                onClicked: {
                    stickersContainer.visible = false
                    stickerMarket.visible = true
                    footerContent.visible = false
                }
            }

            StatusStickerPackIconWithIndicator {
                id: btnHistory
                width: 24
                height: 24
                selected: true
                useIconInsteadOfImage: true
                source: "../../app/img/history_icon.svg"
                anchors.left: btnAddStickerPack.right
                anchors.leftMargin: Style.current.padding
                onClicked: {
                    btnHistory.selected = true
                    stickerPackListView.selectedPackId = -1
                    stickerGrid.model.clear();
                    for(let i = 0; i < StatusSettings.RecentStickers.length; i++){
                        stickerGrid.model.append({
                            ipfsHash: StatusSettings.RecentStickers[i].hash
                        });
                    }
                }
            }


            ScrollView {
                anchors.top: parent.top
                anchors.left: btnHistory.right
                anchors.leftMargin: Style.current.padding
                anchors.right: parent.right
                height: 32
                clip: true
                id: installedStickersSV
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                RowLayout {
                    id: stickersRowLayout
                    spacing: Style.current.padding
                    ListModel {
                        id: installedStickers
                    }
                    Repeater {
                        id: stickerPackListView
                        property int selectedPackId: -1
                        model: {                           
                            installedStickers.clear();
                            Object.keys(StatusSettings.InstalledStickerPacks).forEach(packId => {
                                installedStickers.append({
                                    "packId": parseInt(packId),
                                    "thumbnail": StatusSettings.InstalledStickerPacks[packId].thumbnail,
                                    "stickers": StatusSettings.InstalledStickerPacks[packId].stickers
                                });
                            });
                            return installedStickers;
                        }
                        delegate: StatusStickerPackIconWithIndicator {
                            id: packIconWithIndicator
                            visible: true
                            width: 24
                            height: 24
                            selected: stickerPackListView.selectedPackId === packId
                            source: "image://ipfs-cache/" + thumbnail
                            Layout.preferredHeight: height
                            Layout.preferredWidth: width
                            onClicked: {
                                btnHistory.selected = false
                                stickerPackListView.selectedPackId = packId
                                stickerGrid.model.clear();
                                for(let i = 0; i < StatusSettings.InstalledStickerPacks[packId].stickers.length; i++){
                                    stickerGrid.model.append({
                                        ipfsHash: StatusSettings.InstalledStickerPacks[packId].stickers[i]
                                    });
                                }
                            }
                        }
                    }
                    Repeater {
                        id: loadingStickerPackListView
                        model: !stickerPacksLoaded ? new Array(7) : []

                        delegate: Rectangle {
                            width: 24
                            height: 24
                            Layout.preferredHeight: height
                            Layout.preferredWidth: width
                            radius: width / 2
                            color: Style.current.backgroundHover
                        }
                    }
                }
            }
        }
    }

    /* TODO:
    Connections {
        target: chatsModel.stickers
        onStickerPacksLoaded: {
            stickerPackListView.visible = true
            noStickerPacks.visible = installedPacksCount === 0 || chatsModel.stickers.recent.rowCount() === 0
        }
    }*/
}

