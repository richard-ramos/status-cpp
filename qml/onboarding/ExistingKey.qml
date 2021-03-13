import QtQuick 2.13
import QtQuick.Controls 2.13
import "../imports"

Item {
    property var onClosed: function () {}
    id: root
    anchors.fill: parent

    property string account: ""

    Component.onCompleted: {
        enterSeedPhraseModal.open()
    }

    EnterSeedPhraseModal {
        property bool wentNext: false
        id: enterSeedPhraseModal
        onConfirmSeedClick: function (mnemonic) {
            error = "";
            
            if(!Utils.isMnemonic(mnemonic)){
                //% "Invalid seed phrase"
                error = qsTrId("custom-seed-phrase")
            } else {
                error = onboardingModel.validateMnemonic(mnemonic.trim())
            }

            if (error != "") {
              errorSound.play()
            } else {
              wentNext = true
              enterSeedPhraseModal.close()
              root.account = onboardingModel.importMnemonic(mnemonic)
              removeMnemonicAfterLogin = true
              openPopup(recoverySuccessModalComponent)
            }
        }
        onClosed: function () {
            if (!wentNext) {
                root.onClosed()
            }
        }
    }

    Component {
        id: recoverySuccessModalComponent
        MnemonicRecoverySuccessModal {
            onButtonClicked: {
                createPasswordModal.open();
                close();
            }
            onClosed: function () {
                if (!enterSeedPhraseModal.wentNext) {
                    root.onClosed()
                }
                destroy();
            }
        }
    }
    
    CreatePasswordModal {
        id: createPasswordModal
        account: root.account
        onClosed: function () {
            root.onClosed()
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
