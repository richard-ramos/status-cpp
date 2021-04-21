import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import "../imports"
import im.status.desktop 1.0

Item {
    id: root
    property bool isPending: false
    readonly property string uuid: Utils.uuid()
    property int debounceDelay: 600
    readonly property var validateAsync: Backpressure.debounce(inpAddress, debounceDelay, function (inputValue) {
        root.isPending = true
        var name = inputValue.startsWith("@") ? inputValue.substring(1) : inputValue
        EnsUtils.address(name, function(address){
            root.isPending = false;
            root.resolved(name, address)
        });
    });

    signal resolved(string name, string resolvedAddress)

    function resolveEns(name) {
        if (Utils.isValidEns(name)) {
            root.validateAsync(name)
        }
    }
    width: 12
    height: 12

    Loader {
        anchors.fill: parent
        sourceComponent: loadingIndicator
        active: root.isPending
    }

    Component {
        id: loadingIndicator
        LoadingImage {
            width: root.width
            height: root.height
        }
    }
}
