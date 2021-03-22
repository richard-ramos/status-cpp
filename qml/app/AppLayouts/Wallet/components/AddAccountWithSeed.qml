import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Dialogs 1.3
import "../../../../imports"
import "../../../../shared"
import "../../../../shared/status"

ModalPopup {
    id: popup
    height: 600

    property int marginBetweenInputs: 38
    property string passwordValidationError: ""
    property string seedValidationError: ""
    property string accountNameValidationError: ""
    property bool loading: false

    function validate() {
        if (passwordInput.text === "") {
            //% "You need to enter a password"
            passwordValidationError = qsTrId("you-need-to-enter-a-password")
        } else if (passwordInput.text.length < 6) {
            //% "Password needs to be 6 characters or more"
            passwordValidationError = qsTrId("password-needs-to-be-6-characters-or-more")
        } else {
            passwordValidationError = ""
        }

        if (accountNameInput.text === "") {
            //% "You need to enter an account name"
            accountNameValidationError = qsTrId("you-need-to-enter-an-account-name")
        } else {
            accountNameValidationError = ""
        }

        if (accountSeedInput.text === "") {
            //% "You need to enter a seed phrase"
            seedValidationError = qsTrId("you-need-to-enter-a-seed-phrase")
        } else if (!Utils.isMnemonic(accountSeedInput.text)) {
            //% "Enter a valid mnemonic"
            seedValidationError = qsTrId("enter-a-valid-mnemonic")
        } else {
            seedValidationError = ""
        }

        return passwordValidationError === "" && seedValidationError === "" && accountNameValidationError === ""
    }

    onOpened: {
        accountSeedInput.text = "";
        passwordInput.text = "";
        accountNameInput.text = "";
        passwordValidationError = "";
        seedValidationError = "";
        accountNameValidationError = "";
        accountColorInput.selectedColor = Constants.accountColors[Math.floor(Math.random() * Constants.accountColors.length)]
        passwordInput.forceActiveFocus(Qt.MouseFocusReason)
    }

    onClosed: {
        destroy();
    }

    //% "Add account with a seed phrase"
    title: qsTrId("add-seed-account")

    Input {
        id: passwordInput
        //% "Enter your password…"
        placeholderText: qsTrId("enter-your-password…")
        //% "Password"
        label: qsTrId("password")
        textField.echoMode: TextInput.Password
        validationError: popup.passwordValidationError
    }


    StyledTextArea {
        id: accountSeedInput
        anchors.top: passwordInput.bottom
        anchors.topMargin: marginBetweenInputs
        //% "Enter your seed phrase, separate words with commas or spaces..."
        placeholderText: qsTrId("enter-your-seed-phrase,-separate-words-with-commas-or-spaces...")
        //% "Seed phrase"
        label: qsTrId("recovery-phrase")
        customHeight: 88
        validationError: popup.seedValidationError
    }

    StyledText {
        text: Utils.seedPhraseWordCountText(accountSeedInput.text)
        anchors.right: parent.right
        anchors.top: accountSeedInput.bottom
    }

    Input {
        id: accountNameInput
        anchors.top: accountSeedInput.bottom
        anchors.topMargin: marginBetweenInputs
        //% "Enter an account name..."
        placeholderText: qsTrId("enter-an-account-name...")
        //% "Account name"
        label: qsTrId("account-name")
        validationError: popup.accountNameValidationError
    }

    StatusWalletColorSelect {
        id: accountColorInput
        model: Constants.accountColors
        anchors.top: accountNameInput.bottom
        anchors.topMargin: marginBetweenInputs
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Item {
        Connections {
            target: walletModel
            onInvalidPassword: {
                loading = false;
                errorSound.play()
                accountError.text = qsTr("Invalid password");
                return accountError.open();
            }
            onAccountCreated: {
                if(success){
                    popup.close();
                } else {
                    loading = false;
                    errorSound.play()
                    accountError.text = qsTr("Could not generate an account");
                    return accountError.open();
                }
            }
        }
    }

    footer: StatusButton {
        anchors.top: parent.top
        anchors.right: parent.right
        text: loading ?
        //% "Loading..."
        qsTrId("loading") :
        //% "Add account"
        qsTrId("add-account")

        enabled: !loading && passwordInput.text !== "" && accountNameInput.text !== "" && accountSeedInput.text !== ""

        MessageDialog {
            id: accountError
            title: "Adding the account failed"
            icon: StandardIcon.Critical
            standardButtons: StandardButton.Ok
        }

        onClicked : {
            // TODO the loaidng doesn't work because the function freezes th eview. Might need to use threads
            loading = true
            if (!validate()) {
                errorSound.play()
                return loading = false
            }

            walletModel.addAccountFromSeed(accountSeedInput.text, passwordInput.text, accountNameInput.text, accountColorInput.selectedColor)
            loading = false
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#ffffff";height:500;width:400}
}
##^##*/
