#include "settings.hpp"
#include "libstatus.h"
#include "status.hpp"
#include "utils.hpp"
#include <QCoreApplication>
#include <QDebug>
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

void Settings::saveSettings(SettingTypes setting, int value)
{
	QJsonObject obj{{"method", "settings_saveSetting"}, {"params", QJsonArray{settingsMap[setting], value}}};
	const char* result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
	qDebug() << "SAVE SETTING RESULT" << result;
}

void Settings::saveSettings(SettingTypes setting, QString value)
{
	QJsonObject obj{{"method", "settings_saveSetting"}, {"params", QJsonArray{settingsMap[setting], value}}};
	const char* result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
	qDebug() << "SAVE SETTING RESULT" << result;
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