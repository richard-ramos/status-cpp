#pragma once

#include "mailserver-cycle.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QTimer>
#include <QTranslator>
#include <map>

class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString PublicKey READ publicKey CONSTANT)
	Q_PROPERTY(QString KeyUID READ keyUID CONSTANT)
	Q_PROPERTY(QString Mnemonic READ mnemonic NOTIFY mnemonicRemoved)
	Q_PROPERTY(bool isMnemonicBackedUp READ isMnemonicBackedUp NOTIFY mnemonicRemoved)
	Q_PROPERTY(QString Currency READ currency WRITE setCurrency NOTIFY currencyChanged)
	Q_PROPERTY(QString PreferredUsername READ preferredName WRITE setPreferredName NOTIFY preferredNameChanged)
	Q_PROPERTY(int Appearance READ appearance WRITE setAppearance NOTIFY appearanceChanged)
	Q_PROPERTY(QString CurrentNetwork READ currentNetwork WRITE setCurrentNetwork NOTIFY currentNetworkChanged)
	Q_PROPERTY(QString Fleet READ fleet WRITE setFleet NOTIFY fleetChanged)
	Q_PROPERTY(QString SigningPhrase READ signingPhrase CONSTANT)
	Q_PROPERTY(QJsonObject InstalledStickerPacks READ installedStickerPacks WRITE setInstalledStickerPacks NOTIFY installedStickerPacksChanged)
	Q_PROPERTY(QJsonArray RecentStickers READ recentStickers WRITE setRecentStickers NOTIFY recentStickersChanged)

public:
	static Settings* instance();
	~Settings();

	Q_INVOKABLE void init(QString loginError);
	Q_INVOKABLE void terminate();
	Q_INVOKABLE void removeMnemonic();
	Q_INVOKABLE QString getLinkPreviewWhitelist() const;
	Q_INVOKABLE void changeLocale(QString locale);

	// TODO: move this to mailserver model
	MailserverCycle mailserverCycle;
signals:
	void mailserverRequestSent();
	void localeChanged();

public:
	enum SettingTypes
	{
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
		{Appearance, "appearance"},
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

	QString publicKey() const;
	QString keyUID() const;
	QString mnemonic() const;
	QString currency() const;
	QString preferredName() const;
	QString currentNetwork() const;
	QJsonArray networks() const;
	QString fleet() const;
	QJsonObject installedStickerPacks() const;
	QJsonArray recentStickers() const;
	QString walletRootAddress() const;
	int appearance() const;
	QVector<QString> usernames() const;
	bool isMnemonicBackedUp() const;
	QString signingPhrase() const;
	QString installationId() const;

	void setCurrency(const QString& value);
	void setPreferredName(const QString& value);
	void setCurrentNetwork(const QString& value);
	void setNetworks(const QJsonArray& value);
	void setFleet(const QString& value);
	void setInstalledStickerPacks(const QJsonObject& packs);
	void setRecentStickers(const QJsonArray& packs);
	void setAppearance(int value);
	void setUsernames(QVector<QString> value);

	QJsonObject getNodeConfig() const;

	void addRecentSticker(int packId, QString stickerHash);
	void removeRecentStickerPack(int packId);

signals:
	void initialized();
	void currencyChanged();
	void preferredNameChanged();
	void appearanceChanged();
	void mnemonicRemoved();
	void currentNetworkChanged();
	void fleetChanged();
	void networksChanged();
	void usernamesChanged();
	void installedStickerPacksChanged();
	void recentStickersChanged();

private:
	static Settings* theInstance;
	explicit Settings(QObject* parent = nullptr);

	bool m_initialized;
	QString m_currency;
	QString m_publicKey;
	QString m_keyUID;
	QString m_preferredName;
	QString m_mnemonic;
	QString m_currentNetwork;
	QString m_installationId;
	QString m_fleet;
	QString m_walletRootAddress;
	QJsonArray m_networks;
	QString m_signingPhrase;
	QVector<QString> m_usernames;
	QJsonObject m_installedStickers;
	QJsonArray m_recentStickers;

	int m_appearance;

	mutable QReadWriteLock lock;


	void saveSettings(SettingTypes setting, const QJsonArray& value);
	void saveSettings(SettingTypes setting, const QJsonObject& value);
	void saveSettings(SettingTypes setting, const QVector<QString>& value);
	
	bool saveSetting(SettingTypes setting, QString& member, const QString& value);
	bool saveSetting(SettingTypes setting, int& member, const int& value);

	void save(const QJsonArray& input);

	QTimer* timer;

	QTranslator* m_translator;

public:
	Q_INVOKABLE void startMailserverCycle();
};
