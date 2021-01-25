#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QCryptographicHash>
#include <QUuid>
#include <QRandomGenerator>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <algorithm>
#include <array>

#include "onboarding-model.hpp"
#include "signing-phrases.hpp"
#include "libstatus.h"
#include "utils.hpp"

OnboardingModel::OnboardingModel(QObject * parent): QAbstractListModel(parent)
{
}

QVector<GeneratedAccount> multiAccountGenerateAndDeriveAddresses()
{
    QJsonArray pathsArr = {pathWalletRoot, pathEip1581, pathWhisper, pathDefaultWallet};
    QJsonObject obj
    {
        {"n", 5},
        {"mnemonicPhraseLength", 12},
        {"bip32Passphrase", ""},
        {"paths", pathsArr}

    };
    const char * result = MultiAccountGenerateAndDeriveAddresses(Utils::jsonToStr(obj).toUtf8().data()); // TODO: clear from memory    
    QJsonArray multiAccounts = QJsonDocument::fromJson(result).array();

    QVector<GeneratedAccount> vector;
    foreach (const QJsonValue & value, multiAccounts) {
        const QJsonObject obj = value.toObject();
        GeneratedAccount acc = {};
        acc.id = obj["id"].toString();
        acc.address = obj["address"].toString();
        acc.keyUid = obj["keyUid"].toString();
        acc.mnemonic = obj["mnemonic"].toString(); // TODO: clear from memory?
        acc.publicKey = obj["publicKey"].toString();
        acc.derivedKeys = {
            {pathWalletRoot,    { obj["derived"][pathWalletRoot]["publicKey"].toString(),    obj["derived"][pathWalletRoot]["address"].toString()}},
            {pathEip1581,       { obj["derived"][pathEip1581]["publicKey"].toString(),       obj["derived"][pathEip1581]["address"].toString()}},
            {pathWhisper,       { obj["derived"][pathWhisper]["publicKey"].toString(),       obj["derived"][pathWhisper]["address"].toString()}},
            {pathDefaultWallet, { obj["derived"][pathDefaultWallet]["publicKey"].toString(), obj["derived"][pathDefaultWallet]["address"].toString()}}
        };
        vector.append(acc);
    }
    return vector;
}

void OnboardingModel::populate()
{
    beginResetModel();
    mData.clear();
    mData << multiAccountGenerateAndDeriveAddresses();
    endResetModel();
}

// TODO: destructor. Clear

QHash<int, QByteArray> OnboardingModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id] = "id";
    roles[PublicKey] = "publicKey";
    return roles;
}

int OnboardingModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return mData.size();
}

QVariant OnboardingModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role)
    {
        case Id: return QVariant(mData[index.row()].id);
        case PublicKey: return QVariant(mData[index.row()].publicKey);
    }

    return QVariant();
}

QString OnboardingModel::getAccountId(int index)
{
    return mData[index].id; 
}


void storeDerivedAccount(QString& accountId, QString& hashedPassword)
{
    QJsonObject obj
    {
        {"accountID", accountId},
        {"paths", QJsonArray { pathWalletRoot, pathEip1581, pathWhisper, pathDefaultWallet }},
        {"password", hashedPassword}
    };
    
    MultiAccountStoreDerivedAccounts(Utils::jsonToStr(obj).toUtf8().data());

    // TODO: clear obj
}


QJsonObject getAccountData(GeneratedAccount* account)
{
    return QJsonObject {
         {"name", Utils::generateAlias(account->derivedKeys[pathWhisper].publicKey)},
         {"address", account->address},
         {"photo-path", Utils::generateIdenticon(account->derivedKeys[pathWhisper].publicKey)},
         {"identicon", Utils::generateIdenticon(account->derivedKeys[pathWhisper].publicKey)},
         {"key-uid", account->keyUid},
         {"keycard-pairing", QJsonValue()}
    };

}

QString generateSigningPhrase(int count){
    QStringList words;
    for(int i = 0 ; i < count; i++){
        words.append(phrases[QRandomGenerator::global()->bounded(static_cast<int>(phrases.size()))]);
    }
    return words.join(" ");
}


QJsonObject getAccountSettings(GeneratedAccount* account, QString installationId)
{
    QFile defaultNetworks("../resources/default-networks.json");
    defaultNetworks.open(QIODevice::ReadOnly);

    QString defaultNetworksContent = defaultNetworks.readAll();
    defaultNetworksContent = defaultNetworksContent.replace("%INFURA_KEY%", INFURA_KEY);

    QJsonArray defaultNetworksJson = QJsonDocument::fromJson(defaultNetworksContent.toUtf8()).array();

    return QJsonObject {
        {"key-uid", account->keyUid},
        {"mnemonic", account->mnemonic},
        {"public-key", account->derivedKeys[pathWhisper].publicKey},
        {"name", Utils::generateAlias(account->publicKey)},
        {"address", account->address},
        {"eip1581-address", account->derivedKeys[pathEip1581].address},
        {"dapps-address", account->derivedKeys[pathDefaultWallet].address},
        {"wallet-root-address", account->derivedKeys[pathDefaultWallet].address},
        {"preview-privacy?", true},
        {"signing-phrase", generateSigningPhrase(3)},
        {"log-level", "INFO"},
        {"latest-derived-path", 0},
        {"networks/networks", defaultNetworksJson},
        {"currency", "usd"},
        {"identicon", Utils::generateIdenticon(account->derivedKeys[pathWhisper].publicKey)},
        {"waku-enabled", true},
        {"wallet/visible-tokens", {
            {"mainnet", QJsonArray{ "SNT" }}
          }
        },
        {"appearance", 0},
        {"networks/current-network", DEFAULT_NETWORK_NAME},
        {"installation-id", installationId}
    };

    // TODO: clear mnemonic?
}


QJsonObject getNodeConfig(QString installationId)
{
    QFile nodeConfig("../resources/node-config.json");
    nodeConfig.open(QIODevice::ReadOnly);

    QString nodeConfigContent = nodeConfig.readAll();

    nodeConfigContent = nodeConfigContent.replace("%INSTALLATIONID%", installationId);
    nodeConfigContent = nodeConfigContent.replace("%INFURA_KEY%", INFURA_KEY);

    QJsonObject nodeConfigJson = QJsonDocument::fromJson(nodeConfigContent.toUtf8()).object();

    QFile fleets("../resources/fleets.json");
    fleets.open(QIODevice::ReadOnly);
    QJsonObject fleetJson = QJsonDocument::fromJson(fleets.readAll()).object()["fleets"].toObject()["eth.prod"].toObject();

    QJsonObject clusterConfig = nodeConfigJson["ClusterConfig"].toObject();

    auto boot = fleetJson["boot"].toObject();
    QJsonArray bootNodes;
    for (auto it = boot.begin(); it != boot.end(); ++it) bootNodes << *it;
    clusterConfig["BootNodes"] = bootNodes;
    
    auto rendezvous = fleetJson["rendezvous"].toObject();
    QJsonArray rendezvousNodes;
    for (auto it = rendezvous.begin(); it != rendezvous.end(); ++it) rendezvousNodes << *it;
    clusterConfig["RendezvousNodes"] = rendezvousNodes;

    auto whisper = fleetJson["whisper"].toObject();
    QJsonArray staticNodes;
    for (auto it = whisper.begin(); it != whisper.end(); ++it) staticNodes << *it;
    clusterConfig["StaticNodes"] = staticNodes;

    auto mail = fleetJson["mail"].toObject();
    QJsonArray trustedMailserverNodes;
    for (auto it = mail.begin(); it != mail.end(); ++it) trustedMailserverNodes << *it;
    clusterConfig["TrustedMailServers"] = trustedMailserverNodes;
    
    nodeConfigJson["ClusterConfig"] = clusterConfig;

    return nodeConfigJson;
}


QJsonArray getSubAccountData(GeneratedAccount* account)
{
    return QJsonArray {
        QJsonObject {
            {"public-key", account->derivedKeys[pathDefaultWallet].publicKey},
            {"address", account->derivedKeys[pathDefaultWallet].address},
            {"color", "#4360df"},
            {"wallet", true},
            {"path", pathDefaultWallet},
            {"name", "Status account"}
        },
        QJsonObject {
            {"public-key", account->derivedKeys[pathWhisper].publicKey},
            {"address", account->derivedKeys[pathWhisper].address},
            {"path", pathWhisper},
            {"name", Utils::generateAlias(account->derivedKeys[pathWhisper].publicKey)},
            {"identicon", Utils::generateIdenticon(account->derivedKeys[pathWhisper].publicKey)},
            {"chat", true}
        }
    };
}


bool saveAccountAndLogin(GeneratedAccount *genAccount, QString password)
{
    QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256)); 

    storeDerivedAccount(genAccount->id, hashedPassword);

    QString installationId( QUuid::createUuid().toString(QUuid::WithoutBraces) );

    QJsonObject accountData( getAccountData(genAccount) );
    QJsonObject settings( getAccountSettings(genAccount, installationId) );
    QJsonObject nodeConfig( getNodeConfig(installationId) );
    QJsonArray subAccountData( getSubAccountData(genAccount) );
    
    qDebug() << SaveAccountAndLogin(
                    Utils::jsonToStr(accountData).toUtf8().data(),
                    hashedPassword.toUtf8().data(),
                    Utils::jsonToStr(settings).toUtf8().data(),
                    Utils::jsonToStr(nodeConfig).toUtf8().data(),
                    Utils::jsonToStr(subAccountData).toUtf8().data()
                );
    
    return true;
    // TODO: clear hashedPassword, genAccount
}

QString OnboardingModel::validateMnemonic(QString mnemonic)
{
    // TODO: clear memory
    const char* result(ValidateMnemonic(mnemonic.toUtf8().data()));
    const QJsonObject obj = QJsonDocument::fromJson(result).object();
    return obj["error"].toString();
}


void OnboardingModel::setup(QString accountId, QString password)
{
    auto genAccount = std::find_if(mData.begin(), mData.end(), [accountId](const GeneratedAccount& m) -> bool { return m.id == accountId; });
    if(genAccount != mData.end())
    {
        QtConcurrent::run([=]{
            bool result(saveAccountAndLogin(genAccount, password));

            // TODO: clear mnemonic from memory in list


            emit accountSaved(result);
        });
    }



        // TODO: error handling
        /*

  if error == "":
    debug "Account saved succesfully"
    result = account.toAccount
    return

  raise newException(StatusGoException, "Error saving account and logging in: " & error)*/



    
    // TODO: clear password
}