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
#include <QReadWriteLock>
#include <QTimer>

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
	// TODO: can login signal have an error?
	QObject::connect(Status::instance(), &Status::login, this, &Settings::init);
	QObject::connect(Status::instance(), &Status::logout, this, &Settings::terminate);

	timer = new QTimer(this);
	QObject::connect(this, &Settings::initialized, this, &Settings::startMailserverCycle);
	QObject::connect(Status::instance(), &Status::logout, timer, &QTimer::stop);
}

void Settings::startMailserverCycle()
{
	// Fire immediately
	mailserverCycle.work();
	Settings::instance()->setCurrency("DOP");

	// Execute every 1 seconds
	QObject::connect(timer, &QTimer::timeout, &mailserverCycle, &MailserverCycle::work);
	timer->start(1000);
	Settings::instance()->setCurrency("USD");
}

Settings::~Settings()
{
	terminate();
}

void Settings::init()
{
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
	qDebug() << settings;

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
		return QString();
	QString result(m_publicKey);
	lock.unlock();
	return result;
}

QString Settings::keyUID()
{
	lock.lockForRead();
	if(!m_initialized)
		return QString();
	QString result(m_keyUID);
	lock.unlock();
	return result;
}

QString Settings::mnemonic()
{
	lock.lockForRead();
	if(!m_initialized)
		return QString();
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
		return QString();
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
		emit preferredNameChanged();
	}
	lock.unlock();
}

QString Settings::preferredName()
{
	lock.lockForRead();
	if(!m_initialized)
		return QString();
	QString result(m_preferredName);
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
		return 0;
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
		return false;
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

QString Settings::currentNetwork()
{
	lock.lockForRead();
	if(!m_initialized)
		return 0;
	QString result(m_currentNetwork);
	lock.unlock();
	return result;
}