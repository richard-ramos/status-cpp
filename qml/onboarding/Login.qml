import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.13
import im.status.desktop 1.0
import "../shared"
import "../shared/status"
import "../imports"
import "./Login"

Item {
    property var onGenKeyClicked: function () {}
    property var onExistingKeyClicked: function () {}
    property bool loading: false

    id: loginView
    anchors.fill: parent

    Component.onCompleted: {
        txtPassword.forceActiveFocus(Qt.MouseFocusReason)
    }

    Item {
        id: element
        width: 360
        height: 200
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        StatusImageIdenticon {
            id: userImage
            anchors.horizontalCenter: parent.horizontalCenter
            source: loginModel.currentAccount.image || loginModel.currentAccount.identicon 
        }

        StyledText {
            id: usernameText
            text: loginModel.currentAccount.name
            font.weight: Font.Bold
            font.pixelSize: 17
            anchors.top: userImage.bottom
            anchors.topMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter
        }

        ConfirmAddExistingKeyModal {
            id: confirmAddExstingKeyModal
            onOpenModalClick: function () {
                onExistingKeyClicked()
            }
        }

        SelectAnotherAccountModal {
            id: selectAnotherAccountModal
            onAccountSelect: function (index) {
                loginModel.setSelectedAccount(index)
            }
            onOpenModalClick: function () {
                onExistingKeyClicked()
            }
        }

        Rectangle {
            property bool isHovered: false
            id: changeAccountBtn
            width: 24
            height: 24
            anchors.left: usernameText.right
            anchors.leftMargin: 4
            anchors.verticalCenter: usernameText.verticalCenter

            color: isHovered ? Style.current.backgroundHover : Style.current.transparent

            radius: 4

            SVGImage {
                id: caretImg
                width: 10
                height: 6
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../app/img/caret.svg"
                fillMode: Image.PreserveAspectFit
            }
            ColorOverlay {
                anchors.fill: caretImg
                source: caretImg
                color: Style.current.secondaryText
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onEntered: {
                    changeAccountBtn.isHovered = true
                }
                onExited: {
                    changeAccountBtn.isHovered = false
                }
                onClicked: {
                    if (loginModel.rowCount() > 1) {
                        selectAnotherAccountModal.open()
                    } else {
                        confirmAddExstingKeyModal.open()
                    }
                }
            }
        }

        Address {
            id: publicKeyTxt
            width: 90
            text: loginModel.currentAccount.publicKey
            font.pixelSize: 15
            anchors.top: usernameText.bottom
            anchors.topMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Input {
            id: txtPassword
            anchors.top: publicKeyTxt.bottom
            anchors.topMargin: Style.current.padding * 2
            //% "Enter password"
            placeholderText: qsTrId("enter-password")
            textField.echoMode: TextInput.Password
            textField.focus: true
            Keys.onReturnPressed: {
                submitBtn.clicked()
            }
        }
        StatusRoundButton {
            id: submitBtn
            size: "medium"
            icon.name: "arrow-right"
            icon.width: 18
            icon.height: 14
            visible: txtPassword.text.length > 0
            anchors.left: txtPassword.right
            anchors.leftMargin: Style.current.padding
            anchors.verticalCenter: txtPassword.verticalCenter
            state: loading ? "pending" : "default"
            onClicked: {
                if (loading) {
                    return;
                }
                loading = true
                loginModel.login(txtPassword.textField.text)
            }
        }

        MessageDialog {
            id: loginError
            title: qsTr("Login failed")
            text: qsTr("Login failed. Please re-enter your password and try again.")
            icon: StandardIcon.Critical
            standardButtons: StandardButton.Ok
            onAccepted: {
                txtPassword.textField.clear()
                txtPassword.textField.focus = true
                loading = false
            }
        }

        Connections {
            target: Status
            ignoreUnknownSignals: true
            onLogin: {
                if (error) {
                    errorSound.play()
                    loginError.open()
                }
            }
        }

        Connections {
            target: loginModel
            ignoreUnknownSignals: true
            onLoginError: {
                errorSound.play()
                loginError.open()
            }
        }

        StatusButton {
            id: generateKeysLinkText
            //% "Generate new keys"
            text: qsTrId("generate-new-keys")
            anchors.top: txtPassword.bottom
            anchors.topMargin: 26
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 13
            onClicked: {
                onGenKeyClicked()
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorColor:"#ffffff";formeditorZoom:0.75;height:480;width:640}
}
##^##*/
