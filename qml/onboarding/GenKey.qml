import QtQuick 2.13
import im.status.desktop 1.0

Item {
    id: root

    property var onClosed: function () {}
    
    property string account: ""

    anchors.fill: parent

    Component.onCompleted: {
        genKeyModal.open()
    }

    OnboardingModel {
        id: onboardingModel
    }

    GenKeyModal {
        property bool wentNext: false
        id: genKeyModal
        onOpened: {
            onboardingModel.populate();
        }
        onNextClick: function (selectedIndex) {
            wentNext = true;
            root.account = genKeyModal.getSelectedAccount();
            createPasswordModal.open();
        }
        onClosed: function () {
            if (!wentNext) {
                root.onClosed()
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
    D{i:0;autoSize:true;formeditorColor:"#ffffff";height:480;width:640}
}
##^##*/
