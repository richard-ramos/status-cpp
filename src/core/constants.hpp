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

// TODO: Only mainnet. make it work on multiple networks
const QString StickersAddress("0x0577215622f43a39f4bc9640806dfea9b10d2a36");
const QString StickerMarketAddress("0x12824271339304d3a9f7e096e62a2a7e73b4a7e7");
const QString StickerPackAddress("0x110101156e8F0743948B2A61aFcf3994A8Fb172e");

const QString StatusDomain(".stateofus.eth");
const QString StatusIPFS("https://ipfs.status.im/ipfs/");

const int MaxImageSize = 2000;

QString applicationPath(QString path = "");
QString tmpPath(QString path = "");
QString settingsPath();
QString getTimelineChatId(QString pubkey = "");
QString cachePath(QString path = "");


} // namespace Constants
