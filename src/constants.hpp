#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants
{
    const QString DataDir = "/datadir";
  

    const QString PathWalletRoot("m/44'/60'/0'/0");
    const QString PathEip1581("m/43'/60'/1581'");
    const QString PathDefaultWallet(PathWalletRoot + "/0");
    const QString PathWhisper(PathEip1581  + "/0'/0");


    const QString DefaultNetworkName("mainnet_rpc");


}
#endif
