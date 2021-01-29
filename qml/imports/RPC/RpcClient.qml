pragma Singleton

import QtQuick 2.13
import im.status.desktop 1.0

QtObject {
    function call(method, params, cb){
        if(cb){
            const response = Status.callPrivateRPC(method, params, function(response){
                if(response.error){
                    console.log("CallPrivateRPC error: ", JSON.stringify(response.error));
                    cb(response.error.message);
                    return;
                } else {
                    cb(null, response.result)
                }
            });
        } else {
            const response = Status.callPrivateRPC(method, params);
            if(response.error){
                console.log("CallPrivateRPC error: ", JSON.stringify(response.error));
                throw new Error(response.error.message);
            }
            return response.result;
        } 
    }


}
