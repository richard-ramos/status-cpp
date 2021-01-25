
#include "status.hpp"
#include "libstatus.h"
#include "utils.hpp"

#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QJSEngine>
#include <QVariant>

#include "constants.hpp"


std::map<QString, Status::SignalType> Status::signalMap;
Status *Status::theInstance;

Status *Status::instance()
{
    if (theInstance == 0)
      theInstance = new Status();
    return theInstance;
}


Status::Status(QObject * parent): QObject(parent)
{
  SetSignalEventCallback((void *)&Status::statusGoEventCallback);

  signalMap = {
    {"messages.new", SignalType::Message},
    {"wallet", SignalType::Wallet},
    {"node.ready", SignalType::NodeReady},
    {"node.started", SignalType::NodeStarted},
    {"node.stopped", SignalType::NodeStopped},
    {"node.login", SignalType::NodeLogin},
    {"envelope.sent", SignalType::EnvelopeSent},
    {"envelope.expired", SignalType::EnvelopeExpired},
    {"mailserver.request.completed", SignalType::MailserverRequestCompleted},
    {"mailserver.request.expired", SignalType::MailserverRequestExpired},
    {"discovery.started", SignalType::DiscoveryStarted},
    {"discovery.stopped", SignalType::DiscoveryStopped},
    {"discovery.summary", SignalType::DiscoverySummary},
    {"subscriptions.data", SignalType::SubscriptionsData},
    {"subscriptions.error", SignalType::SubscriptionsError},
    {"whisper.filter.added", SignalType::WhisperFilterAdded}
  };
}


void Status::statusGoEventCallback(const char *event) {
  const QJsonObject signalEvent = QJsonDocument::fromJson(event).object();
  SignalType signalType(Unknown);
  if(!signalMap.count(signalEvent["type"].toString())){
    // TODO: log unknwon signal;
    return;
  }

  signalType = signalMap[signalEvent["type"].toString()];
  
  qDebug() << "Signal received: "<< signalType;
  if(signalType == NodeLogin){
    emit instance()->login(signalEvent["event"]["error"].toString());
  }
}


void Status::logout()
{
  // TODO:
  Logout();
}


QString Status::generateAlias(QString publicKey)
{
  return Utils::generateAlias(publicKey);
}


QString Status::generateIdenticon(QString publicKey)
{
  return Utils::generateIdenticon(publicKey);
}


QString Status::initKeystore()
{
  QString fullDirPath = QCoreApplication::applicationDirPath() + Constants::DataDir; // TODO: set correct path
  const char * initKeystoreResult = InitKeystore(QString(fullDirPath + "/keystore").toUtf8().data());
  return QString(initKeystoreResult);
  // TODO: error handling
}


/*

void Status::multiAccountStoreDerivedAccounts(QString accountId, QString password, const QJSValue &callback) {
    auto *watcher = new QFutureWatcher<QVariantMap>(this);
    QObject::connect(watcher, &QFutureWatcher<QVariantMap>::finished,
                     this, [this,watcher,callback]() {
        QVariantMap result = watcher->result();
        QJSValue cbCopy(callback); // needed as callback is captured as const
        QJSEngine *engine = qjsEngine(this);
        cbCopy.call(QJSValueList { engine->toScriptValue(result) });
        watcher->deleteLater();
    }); 
    watcher->setFuture(QtConcurrent::run(this, &Status::multiAccountStoreDerivedAccounts, accountId, password));
}
*/