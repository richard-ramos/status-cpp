#pragma once

#include <QString>

namespace Constants
{
const QString DataDir = "/datadir";
const QString Keystore = "/datadir/keystore";

const QString PathWalletRoot("m/44'/60'/0'/0");
const QString PathEip1581("m/43'/60'/1581'");
const QString PathDefaultWallet(PathWalletRoot + "/0");
const QString PathWhisper(PathEip1581 + "/0'/0");

const QString DefaultNetworkName("mainnet_rpc");
const QString ZeroAddress("0x0000000000000000000000000000000000000000");

const QString StatusDomain(".stateofus.eth");

const int MaxImageSize = 2000;


QString applicationPath(QString path = "");
QString tmpPath(QString path = "");
QString settingsPath();
QString getTimelineChatId(QString pubkey = "");

} // namespace Constants
