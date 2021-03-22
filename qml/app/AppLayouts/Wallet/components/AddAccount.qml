import QtQuick 2.13
import QtQuick.Controls 2.13
import "../../../../shared"
import "../../../../shared/status"
import "../../../../imports"

StatusRoundButton {
    id: btnAdd
    icon.name: "plusSign"
    pressedIconRotation: 45
    size: "medium"
    width: 36
    height: 36

    onClicked: {
        btnAdd.state = "pressed"
        let x = btnAdd.iconX + btnAdd.icon.width / 2 - newAccountMenu.width / 2
        newAccountMenu.popup(x, btnAdd.icon.height + 10)
    }

    Component {
        id: generateAccountModal
        GenerateAccountModal { }
    }

    Component {
        id: addAccountWithSeedModal
        AddAccountWithSeed { }
    }
    
    Component {
        id: addAccountWithPrivateKeydModal
        AddAccountWithPrivateKey { }
    }

    Component {
        id: addWatchOnlyAccountModal
        AddWatchOnlyAccount { }
    }

    PopupMenu {
        id: newAccountMenu
        width: 260
        Action {
            //% "Generate an account"
            text: qsTrId("generate-a-new-account")
            icon.source: "../../../img/generate_account.svg"
            icon.width: 19
            icon.height: 19
            onTriggered: {
                openPopup(generateAccountModal)
            }
        }
        Action {
            //% "Add a watch-only address"
            text: qsTrId("add-a-watch-account")
            icon.source: "../../../img/add_watch_only.svg"
            icon.width: 19
            icon.height: 19
            onTriggered: {
                openPopup(addWatchOnlyAccountModal)
            }
        }
        Action {
            //% "Enter a seed phrase"
            text: qsTrId("enter-a-seed-phrase")
            icon.source: "../../../img/enter_seed_phrase.svg"
            icon.width: 19
            icon.height: 19
            onTriggered: {
                openPopup(addAccountWithSeedModal)
            }
        }
        Action {
            //% "Enter a private key"
            text: qsTrId("enter-a-private-key")
            icon.source: "../../../img/enter_private_key.svg"
            icon.width: 19
            icon.height: 19
            onTriggered: {
                openPopup(addAccountWithPrivateKeydModal)
            }
        }
        onAboutToHide: {
            btnAdd.state = "default"
        }
    }
}

/*##^##
Designer {
    D{i:0;height:36;width:36}
}
##^##*/
