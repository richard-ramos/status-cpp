#include "settings.hpp"
#include "libstatus.h"
#include "status.hpp"
#include "utils.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QReadWriteLock>
#include <QTimer>
#include <QTranslator>

Settings* Settings::theInstance;

Settings* Settings::instance()
{
	if(theInstance == 0)
		theInstance = new Settings();
	return theInstance;
}

Settings::Settings(QObject* parent)
	: QObject(parent)
{
	m_initialized = false;

	m_translator = new QTranslator();

	// TODO: can login signal have an error?
	QObject::connect(Status::instance(), &Status::login, this, &Settings::init);
	QObject::connect(Status::instance(), &Status::logout, this, &Settings::terminate);

	timer = new QTimer(this);
	QObject::connect(this, &Settings::initialized, this, &Settings::startMailserverCycle);
	QObject::connect(Status::instance(), &Status::logout, timer, &QTimer::stop);
	QObject::connect(Status::instance(), &Status::discoverySummary, &mailserverCycle, &MailserverCycle::peerSummaryChange);
	QObject::connect(&mailserverCycle, &MailserverCycle::requestSent, this, &Settings::mailserverRequestSent);
}

void Settings::startMailserverCycle()
{
	// Fire immediately
	mailserverCycle.work();
	// Execute every 1 seconds
	QObject::connect(timer, &QTimer::timeout, &mailserverCycle, &MailserverCycle::work);
	timer->start(10000);
}

Settings::~Settings()
{
	terminate();
}

void Settings::init(QString loginError)
{
	if(loginError != "")
		return;

	// TODO: extract to callPrivateRPC helper function
	lock.lockForWrite();
	QJsonObject obj{{"method", "settings_getSettings"}, {"params", QJsonArray{}}};
	const char* result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
	// TODO: error handling for callrpc

	const QJsonObject settings = QJsonDocument::fromJson(result).object()["result"].toObject();
	m_publicKey = settings[settingsMap[SettingTypes::PublicKey]].toString();
	m_keyUID = settings[settingsMap[SettingTypes::KeyUID]].toString();
	m_currency = settings[settingsMap[SettingTypes::Currency]].toString();
	m_preferredName = settings[settingsMap[SettingTypes::PreferredUsername]].toString();
	m_mnemonic = settings[settingsMap[SettingTypes::Mnemonic]].toString();
	m_appearance = settings[settingsMap[SettingTypes::Appearance]].toInt();
	m_currentNetwork = settings[settingsMap[SettingTypes::Networks_CurrentNetwork]].toString();
	m_installationId = settings[settingsMap[SettingTypes::InstallationId]].toString();
	m_fleet = settings[settingsMap[SettingTypes::Fleet]].toString();
	m_networks = settings[settingsMap[SettingTypes::Networks_Networks]].toArray();
	m_walletRootAddress = settings[settingsMap[SettingTypes::WalletRootAddress]].toString();
	m_signingPhrase = settings[settingsMap[SettingTypes::SigningPhrase]].toString();
	m_installedStickers = settings[settingsMap[SettingTypes::Stickers_PacksInstalled]].toObject();
	m_recentStickers = settings[settingsMap[SettingTypes::Stickers_Recent]].toArray();

	if(!settings[settingsMap[SettingTypes::Usernames]].isUndefined())
	{
		foreach(const QJsonValue& value, settings[settingsMap[SettingTypes::Usernames]].toArray())
		{
			m_usernames << value.toString();
		}
	}

	// defaults:
	if(m_fleet == "")
		m_fleet = QStringLiteral("eth.prod");

	m_initialized = true;
	lock.unlock();

	emit initialized();
}

void Settings::terminate()
{
	// TODO: clear all settings from memory
	lock.lockForWrite();
	m_initialized = false;
	lock.unlock();
}

// TODO: Probably can be replaced by templates. Research this
void Settings::saveSettings(SettingTypes setting, const int& value)
{
	save(QJsonArray{settingsMap[setting], value});
}

void Settings::saveSettings(SettingTypes setting, const QJsonArray& value)
{
	save(QJsonArray{settingsMap[setting], value});
}

void Settings::saveSettings(SettingTypes setting, const QJsonObject& value)
{
	save(QJsonArray{settingsMap[setting], value});
}

void Settings::saveSettings(SettingTypes setting, const QString& value)
{
	save(QJsonArray{settingsMap[setting], value});
}

void Settings::saveSettings(SettingTypes setting, const QVector<QString>& value)
{
	QJsonArray valueArray;
	foreach(const QString& v, value)
	{
		valueArray << v;
	}
	save(QJsonArray{settingsMap[setting], valueArray});
}

void Settings::save(const QJsonArray& input)
{
	QJsonObject obj{{"method", "settings_saveSetting"}, {"params", input}};
	const char* result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
	// TODO: error handling?
}

QString Settings::publicKey()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}

	QString result(m_publicKey);
	lock.unlock();
	return result;
}

QString Settings::keyUID()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_keyUID);
	lock.unlock();
	return result;
}

QString Settings::mnemonic()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_mnemonic);
	lock.unlock();
	return result;
}

void Settings::setCurrency(const QString& value)
{
	lock.lockForWrite();
	if(value != m_currency)
	{
		m_currency = value;
		saveSettings(SettingTypes::Currency, value);
		emit currencyChanged();
	}
	lock.unlock();
}

QString Settings::currency()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_currency);
	lock.unlock();
	return result;
}

void Settings::setPreferredName(const QString& value)
{
	lock.lockForWrite();
	if(value != m_preferredName)
	{
		m_preferredName = value;
		saveSettings(SettingTypes::PreferredUsername, value);
	}
	lock.unlock();
	emit preferredNameChanged();
}

QString Settings::preferredName()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_preferredName);
	lock.unlock();
	return result;
}

QString Settings::installationId()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_installationId);
	lock.unlock();
	return result;
}

void Settings::setAppearance(int value)
{
	lock.lockForWrite();
	if(value != m_appearance)
	{
		m_appearance = value;
		saveSettings(SettingTypes::Appearance, value);
	}
	lock.unlock();
	emit appearanceChanged();
}

int Settings::appearance()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return 0;
	}
	int result(m_appearance);
	lock.unlock();
	return result;
}

void Settings::removeMnemonic()
{
	lock.lockForWrite();
	m_mnemonic = ""; // TODO: clear from memory
	saveSettings(SettingTypes::Mnemonic, m_mnemonic);
	lock.unlock();

	emit mnemonicRemoved();
}

bool Settings::isMnemonicBackedUp()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return false;
	}
	bool result(m_mnemonic == "");
	lock.unlock();
	return result;
}

QString Settings::getLinkPreviewWhitelist()
{
	QJsonObject obj{{"method", "wakuext_getLinkPreviewWhitelist"}, {"params", QJsonArray{}}};
	const char* result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
	QJsonArray whiteList = QJsonDocument::fromJson(result).object()["result"].toArray();
	return Utils::jsonToStr(whiteList);
}

QJsonObject Settings::getNodeConfig()
{
	// TODO: DRY this code with onboarding

	QFile nodeConfig(":/resources/node-config.json");
	nodeConfig.open(QIODevice::ReadOnly);

	QString nodeConfigContent = nodeConfig.readAll();

	QJsonObject nodeConfigJson = QJsonDocument::fromJson(nodeConfigContent.toUtf8()).object();

	QFile fleets(":/resources/fleets.json");
	fleets.open(QIODevice::ReadOnly);
	QJsonObject fleetJson = QJsonDocument::fromJson(fleets.readAll()).object()["fleets"].toObject()[m_fleet].toObject();

	QJsonObject clusterConfig = nodeConfigJson["ClusterConfig"].toObject();

	auto boot = fleetJson["boot"].toObject();
	QJsonArray bootNodes;
	for(auto it = boot.begin(); it != boot.end(); ++it)
		bootNodes << *it;
	clusterConfig["BootNodes"] = bootNodes;

	auto rendezvous = fleetJson["rendezvous"].toObject();
	QJsonArray rendezvousNodes;
	for(auto it = rendezvous.begin(); it != rendezvous.end(); ++it)
		rendezvousNodes << *it;
	clusterConfig["RendezvousNodes"] = rendezvousNodes;

	auto whisper = fleetJson["whisper"].toObject();
	QJsonArray staticNodes;
	for(auto it = whisper.begin(); it != whisper.end(); ++it)
		staticNodes << *it;
	clusterConfig["StaticNodes"] = staticNodes;

	auto mail = fleetJson["mail"].toObject();
	QJsonArray trustedMailserverNodes;
	for(auto it = mail.begin(); it != mail.end(); ++it)
		trustedMailserverNodes << *it;
	clusterConfig["TrustedMailServers"] = trustedMailserverNodes;

	nodeConfigJson["ClusterConfig"] = clusterConfig;

	QJsonObject ShhextConfig = nodeConfigJson["ShhextConfig"].toObject();

	ShhextConfig["InstallationID"] = m_installationId;

	foreach(const QJsonValue& network, m_networks)
	{
		if(network["id"].toString() != m_currentNetwork)
			continue;

		nodeConfigJson["DataDir"] = network["config"]["DataDir"].toString();
		ShhextConfig["VerifyENSURL"] = network["config"]["UpstreamConfig"]["URL"].toString();
		ShhextConfig["VerifyTransactionChainID"] = network["config"]["NetworkId"].toInt();
		ShhextConfig["VerifyTransactionURL"] = network["config"]["UpstreamConfig"]["URL"].toString();
		nodeConfigJson["ShhextConfig"] = ShhextConfig;

		QJsonObject UpstreamConfig = nodeConfigJson["UpstreamConfig"].toObject();
		UpstreamConfig["URL"] = network["config"]["UpstreamConfig"]["URL"].toString();
		nodeConfigJson["UpstreamConfig"] = UpstreamConfig;
		break;
	}

	return nodeConfigJson;
}

void Settings::setCurrentNetwork(const QString& value)
{
	lock.lockForWrite();
	if(value != m_currentNetwork)
	{
		m_currentNetwork = value;
		saveSettings(SettingTypes::Networks_CurrentNetwork, value);
		saveSettings(SettingTypes::NodeConfig, getNodeConfig());
		saveSettings(SettingTypes::Stickers_PacksInstalled, QJsonArray{});
		saveSettings(SettingTypes::Stickers_Recent, QJsonArray{});
	}
	lock.unlock();
	emit currentNetworkChanged();
}

void Settings::setFleet(const QString& value)
{
	lock.lockForWrite();
	if(value != m_fleet)
	{
		m_fleet = value;
		saveSettings(SettingTypes::Fleet, value);
		saveSettings(SettingTypes::NodeConfig, getNodeConfig());
	}
	lock.unlock();
	emit fleetChanged();
}

QString Settings::currentNetwork()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_currentNetwork);
	lock.unlock();
	return result;
}

QString Settings::fleet()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_fleet);
	lock.unlock();
	return result;
}

void Settings::setNetworks(const QJsonArray& value)
{
	lock.lockForWrite();
	if(value != m_networks)
	{
		m_networks = value;
		saveSettings(SettingTypes::Networks_Networks, value);
	}
	lock.unlock();
	emit networksChanged();
}

QJsonArray Settings::networks()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QJsonArray();
	}
	QJsonArray result(m_networks);
	lock.unlock();
	return result;
}

QVector<QString> Settings::usernames()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QVector<QString>{};
	}
	QVector<QString> result = m_usernames;
	lock.unlock();
	return result;
}

void Settings::setUsernames(QVector<QString> value)
{
	lock.lockForWrite();
	if(value != m_usernames)
	{
		m_usernames = value;
		saveSettings(SettingTypes::Usernames, value);
	}
	lock.unlock();
	emit usernamesChanged();
}

QString Settings::walletRootAddress()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_walletRootAddress);
	lock.unlock();
	return result;
}

QString Settings::signingPhrase()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QString();
	}
	QString result(m_signingPhrase);
	lock.unlock();
	return result;
}

void Settings::changeLocale(QString locale)
{
	if(!m_translator->isEmpty())
	{
		QCoreApplication::removeTranslator(m_translator);
	}

	QString translationPackage = ":/i18n/qml_" + locale + ".qm";
	if(m_translator->load(translationPackage))
	{
		bool success = QCoreApplication::installTranslator(m_translator);
		emit localeChanged();
	}
	else
	{
		qWarning() << "Failed to load translation file: " << translationPackage;
	}
}

void Settings::setInstalledStickerPacks(const QJsonObject& value)
{
	lock.lockForWrite();
	m_installedStickers = value;
	saveSettings(SettingTypes::Stickers_PacksInstalled, value);
	lock.unlock();
	emit installedStickerPacksChanged();
}

QJsonObject Settings::installedStickerPacks()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QJsonObject{};
	}
	QJsonObject result(m_installedStickers);
	lock.unlock();
	return result;
}

void Settings::setRecentStickers(const QJsonArray& value)
{
	lock.lockForWrite();
	m_recentStickers = value;
	saveSettings(SettingTypes::Stickers_Recent, value);
	lock.unlock();
	emit recentStickersChanged();
}

QJsonArray Settings::recentStickers()
{
	lock.lockForRead();
	if(!m_initialized)
	{
		lock.unlock();
		return QJsonArray{};
	}
	QJsonArray result(m_recentStickers);
	lock.unlock();
	return result;
}

void Settings::addRecentSticker(int packId, QString stickerHash)
{
	lock.lockForWrite();

	uint i = -1;
	int found = -1;
	foreach(const QJsonValue& jsonValue, m_recentStickers)
	{
		i++;
		const QJsonObject obj = jsonValue.toObject();
		if(obj["hash"].toString() == stickerHash && obj["packId"].toInt() == packId)
		{
			found = i;
			break;
		}
	}

	if(found > -1)
	{
		m_recentStickers.removeAt(found);
	}

	m_recentStickers.push_front(QJsonObject{{"hash", stickerHash}, {"packId", packId}});
	if(m_recentStickers.count() > 36)
	{
		m_recentStickers.pop_back();
	}

	saveSettings(SettingTypes::Stickers_Recent, m_recentStickers);
	lock.unlock();
	emit recentStickersChanged();
}

void Settings::removeRecentStickerPack(int packId)
{
	lock.lockForWrite();

	uint i = -1;
	int found = -1;

	QVector<int> rowsToRemove;

	foreach(const QJsonValue& jsonValue, m_recentStickers)
	{
		i++;
		if(jsonValue.toObject()["packId"].toInt() == packId)
		{
			rowsToRemove << i;
		}
	}

	foreach(const int& idx, rowsToRemove)
	{
		m_recentStickers.removeAt(idx);
	}

	saveSettings(SettingTypes::Stickers_Recent, m_recentStickers);
	lock.unlock();
	emit recentStickersChanged();
}