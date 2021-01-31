#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QJsonObject>
#include <QReadWriteLock>

#include <map>

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString PublicKey READ publicKey CONSTANT)
    Q_PROPERTY(QString KeyUID READ keyUID CONSTANT)
    Q_PROPERTY(QString Currency READ currency WRITE setCurrency NOTIFY currencyChanged)
    Q_PROPERTY(QString PreferredUsername READ preferredName WRITE setPreferredName NOTIFY preferredNameChanged)

public:
    static Settings *instance();
    ~Settings();

    Q_INVOKABLE void init();
    Q_INVOKABLE void terminate();

    enum SettingTypes {
        Appearance,
        Bookmarks,
        Currency,
        EtherscanLink,
        InstallationId,
        KeyUID,
        Mnemonic,
        Networks_Networks,
        Networks_CurrentNetwork,
        NodeConfig,
        PublicKey,
        DappsAddress,
        Stickers_PacksInstalled,
        Stickers_Recent,
        WalletRootAddress,
        LatestDerivedPath,
        PreferredUsername,
        Usernames,
        SigningPhrase,
        Fleet,
        VisibleTokens,
        PinnedMailservers
    };

    std::map<SettingTypes, QString> settingsMap = {
        {Appearance, "appareance"},
        {Bookmarks, "bookmarks"},
        {KeyUID, "key-uid"},
        {Currency, "currency"},
        {EtherscanLink, "etherscan-link"},
        {InstallationId, "installation-id"},
        {Mnemonic, "mnemonic"},
        {Networks_Networks, "networks/networks"},
        {Networks_CurrentNetwork, "networks/current-network"},
        {NodeConfig, "node-config"},
        {PublicKey, "public-key"},
        {DappsAddress, "dapps-address"},
        {Stickers_PacksInstalled, "stickers/packs-installed"},
        {Stickers_Recent, "stickers/recent-stickers"},
        {WalletRootAddress, "wallet-root-address"},
        {LatestDerivedPath, "latest-derived-path"},
        {PreferredUsername, "preferred-name"},
        {Usernames, "usernames"},
        {SigningPhrase, "signing-phrase"},
        {Fleet, "fleet"},
        {VisibleTokens, "wallet/visible-tokens"},
        {PinnedMailservers, "pinned-mailservers"},
    };


    QString publicKey();
    QString keyUID();


    QString currency();
    void setCurrency(const QString &value);

    QString preferredName();
    void setPreferredName(const QString &value);

    
signals:
    void currencyChanged();
    void preferredNameChanged();


private:
    static Settings *theInstance;
    explicit Settings(QObject * parent = nullptr);

    bool m_initialized;
    QString m_currency;
    QString m_publicKey;
    QString m_keyUID;
    QString m_preferredName;

    QReadWriteLock lock;

    void saveSettings(SettingTypes setting, QString value);
};

#endif // SETTINGS_H

