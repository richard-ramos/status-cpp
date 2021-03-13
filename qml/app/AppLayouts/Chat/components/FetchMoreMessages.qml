import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import "../../../../shared"
import "../../../../shared/status"
import "../../../../imports"

PopupMenu {
    //% "Fetch Messages"
    title: qsTrId("fetch-messages")

    // TODO call fetch for the wanted duration
    //% "Last 24 hours"
    Action {
        text: qsTrId("last-24-hours");
        icon.width: 0;
        onTriggered: {
            chatsModel.get(chat.index).requestMessagesInLast(Constants.fetchRangeLast24Hours);
        }
    }
    //% "Last 2 days"
    Action {
        text: qsTrId("last-2-days");
        icon.width: 0;
        onTriggered: {
            chatsModel.get(chat.index).requestMessagesInLast(Constants.fetchRangeLast2Days);
        }
      }
    //% "Last 3 days"
    Action {
        text: qsTrId("last-3-days");
        icon.width: 0;
        onTriggered: {
            chatsModel.get(chat.index).requestMessagesInLast(Constants.fetchRangeLast3Days);
        }
    }
    //% "Last 7 days"
    Action {
        text: qsTrId("last-7-days");
        icon.width: 0;
        onTriggered: {
            chatsModel.get(chat.index).requestMessagesInLast(Constants.fetchRangeLast7Days);
        }
    }
}
