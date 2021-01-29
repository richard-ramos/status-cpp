import QtQuick 2.13
import im.status.desktop 1.0
import "./"


QtObject {
    property string largeImage;
    property string thumbnail;
    property string identicon;

    function parseResponse(result){
        if(!result) return;
        result.forEach(r => {
            if(r.type === "large"){
                largeImage = r.uri;
            } else {
                thumbnail = r.uri;
            }
        });
    }

    Component.onCompleted: {
        const result = RpcClient.call("multiaccounts_getIdentityImages", [Settings.KeyUID]);
        parseResponse(result);
        identicon = Status.generateIdenticon(Settings.PublicKey);
    }

    function upload(imagePath, aX, aY, bX, bY){
        const path = imagePath.replace(/^(file:\/{2})/,"");
        const result = RpcClient.call("multiaccounts_storeIdentityImage", [Settings.KeyUID, path, aX, aY, bX, bY]);
        parseResponse(result);
    }

    function remove(){
        const result = RpcClient.call("multiaccounts_deleteIdentityImage", [Settings.KeyUID]);
        largeImage = "";
        thumbnail = "";
    }
}
