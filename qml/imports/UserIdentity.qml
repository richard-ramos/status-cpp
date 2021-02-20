pragma Singleton

import QtQuick 2.13
import im.status.desktop 1.0

QtObject {
    readonly property string id: StatusSettings.PublicKey
    // address
    readonly property string name: StatusSettings.PreferredUsername
    readonly property bool ensVerified: true // TODO: check if ens is valid on login
    readonly property string ensVerifiedAt: "0" // TODO
    readonly property string lastENSClockValue: "0" // TODO
    readonly property string ensVerificationRetries: "0"
    readonly property string alias: Status.generateAlias(StatusSettings.PublicKey)
    readonly property string identicon: Status.generateIdenticon(StatusSettings.PublicKey)
    readonly property string lastUpdated: "0"
    readonly property string localNickname: ""
    property string image: identicon
    readonly property bool isAdded: false
    readonly property bool isBlocked: false
}