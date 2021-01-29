#include <QDebug>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QReadWriteLock>

#include "settings.hpp"
#include "status.hpp"
#include "libstatus.h"
#include "utils.hpp"

Settings::Settings(QObject * parent): QObject(parent)
{
    m_initialized = false;
    QObject::connect(Status::instance(), &Status::login, this, &Settings::init); // TODO: check if there is a login error?
    QObject::connect(Status::instance(), &Status::logout, this, &Settings::terminate);
}

Settings::~Settings()
{
    terminate();
}

void Settings::init()
{
    // TODO: extract to callPrivateRPC helper function
    lock.lockForWrite();
    QJsonObject obj
    {
        {"method", "settings_getSettings"},
        {"params", QJsonArray {}}
    };
    const char * result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());  
    // TODO: error handling for callrpc

    const QJsonObject settings = QJsonDocument::fromJson(result).object();
    m_publicKey = settings["result"][settingsMap[SettingTypes::PublicKey]].toString();
    m_keyUID  = settings["result"][settingsMap[SettingTypes::KeyUID]].toString();
    m_currency  = settings["result"][settingsMap[SettingTypes::Currency]].toString();

    m_initialized = true;
    lock.unlock();
}

void Settings::terminate()
{
    // TODO: clear mnemonic from memory
    lock.lockForWrite();
    m_initialized = false;
    lock.unlock();
}


void Settings::saveSettings(SettingTypes setting, QString value)
{
    QJsonObject obj
    {
        {"method", "settings_saveSetting"},
        {"params", QJsonArray {settingsMap[setting], value}}
    };
    const char * result = CallPrivateRPC(Utils::jsonToStr(obj).toUtf8().data());
    qDebug() << "SAVE SETTING RESULT" << result;
}


QString Settings::publicKey()
{
    lock.lockForRead();
    if(!m_initialized) return QString();
    QString result(m_publicKey);
    lock.unlock();
    return result;
}


QString Settings::keyUID()
{
    lock.lockForRead();
    if(!m_initialized) return QString();
    QString result(m_keyUID);
    lock.unlock();
    return result;
}


void Settings::setCurrency(const QString &value)
{
    lock.lockForWrite();
    if (value != m_currency) {
        m_currency = value;
        saveSettings(SettingTypes::Currency, value);
        emit currencyChanged();
    }
    lock.unlock();
}


QString Settings::currency()
{
    lock.lockForRead();
    if(!m_initialized) return QString();
    QString result(m_currency);
    lock.unlock();
    return result;
}