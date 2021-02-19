#pragma once

#include "mailserver-cycle.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QReadWriteLock>
#include <QTimer>
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

public:
	static Settings* instance();
	~Settings();

	Q_INVOKABLE void init();
	Q_INVOKABLE void terminate();
	Q_INVOKABLE void removeMnemonic();
	Q_INVOKABLE QString getLinkPreviewWhitelist();

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

	QString publicKey();
	QString keyUID();
	QString mnemonic();

	QString currency();
	void setCurrency(const QString& value);

	QString preferredName();
	void setPreferredName(const QString& value);

	QString currentNetwork();
	void setCurrentNetwork(const QString& value);

	QString fleet();
	void setFleet(const QString& value);

	int appearance();
	void setAppearance(int value);

	bool isMnemonicBackedUp();

	QJsonObject getNodeConfig();


signals:
	void initialized();
	void currencyChanged();
	void preferredNameChanged();
	void appearanceChanged();
	void mnemonicRemoved();
	void currentNetworkChanged();
	void fleetChanged();

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
	QJsonArray m_networks;

	int m_appearance;

	QReadWriteLock lock;

	void saveSettings(SettingTypes setting, const QString& value);
	void saveSettings(SettingTypes setting, const int& value);
	void saveSettings(SettingTypes setting, const QJsonArray& value);
	void saveSettings(SettingTypes setting, const QJsonObject& value);

	void save(const QJsonArray& input);

	// TODO: move this to mailserver model
	MailserverCycle mailserverCycle;
	QTimer* timer;


public:
	Q_INVOKABLE void startMailserverCycle();
};
