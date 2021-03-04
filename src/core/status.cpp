
#include "status.hpp"
#include "constants.hpp"
#include "libstatus.h"
#include "settings.hpp"
#include "utils.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFuture>
#include <QFutureWatcher>
#include <QJSEngine>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextDocumentFragment>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include "QrCode.hpp"

std::map<QString, Status::SignalType> Status::signalMap;
Status* Status::theInstance;

Status* Status::instance()
{
	if(theInstance == 0)
		theInstance = new Status();
	return theInstance;
}

Status::Status(QObject* parent)
	: QObject(parent)
{
	m_online = false;

	SetSignalEventCallback((void*)&Status::signalCallback);

	signalMap = {{"messages.new", SignalType::Message},
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
				 {"whisper.filter.added", SignalType::WhisperFilterAdded}};
}

void uint64ToStrReplacements(QString& input)
{
	input.replace(QRegularExpression(QStringLiteral("\"lastClockValue\":(\\d+)")), QStringLiteral("\"lastClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"timestamp\":(\\d+)")), QStringLiteral("\"timestamp\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"deletedAtClockValue\":(\\d+)")), QStringLiteral("\"deletedAtClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"clock\":(\\d+)")), QStringLiteral("\"clock\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"clockValue\":(\\d+)")), QStringLiteral("\"clockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"whisperTimestamp\":(\\d+)")), QStringLiteral("\"whisperTimestamp\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"ensVerifiedAt\":(\\d+)")), QStringLiteral("\"ensVerifiedAt\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"lastENSClockValue\":(\\d+)")), QStringLiteral("\"lastENSClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"lastUpdated\":(\\d+)")), QStringLiteral("\"lastUpdated\":\"\\1\""));
}

void Status::processDiscoverySummarySignal(const QJsonObject& signalEvent)
{
	QJsonArray peers(signalEvent["event"].toArray());

	if(peers.count() > 0 && !m_online)
	{
		m_online = true;
		emit onlineStatusChanged(true);
	}
	else if(peers.count() == 0 && m_online)
	{
		m_online = false;
		emit onlineStatusChanged(false);
	}

	QVector<QString> peerVector;
	foreach(const QJsonValue& peer, peers)
		peerVector << peer.toObject()["enode"].toString();
	
	emit discoverySummary(peerVector);
}

void Status::processSignal(QString ev)
{
	// WARNING: Signals are returning bigints as numeric values instead of strings
	uint64ToStrReplacements(ev);

	const QJsonObject signalEvent = QJsonDocument::fromJson(ev.toUtf8()).object();
	SignalType signalType(Unknown);
	if(!signalMap.count(signalEvent["type"].toString()))
	{
		qWarning() << "Unknown signal: " << signalEvent["type"].toString();
		qDebug() << ev;
		return;
	}

	signalType = signalMap[signalEvent["type"].toString()];

	switch(signalType)
	{
	case NodeLogin: emit instance()->login(signalEvent["event"]["error"].toString()); break;
	case NodeReady: emit instance()->nodeReady(signalEvent["event"]["error"].toString()); break;
	case NodeStopped: emit instance()->nodeStopped(signalEvent["event"]["error"].toString()); break;
	case Message: emit instance()->message(signalEvent["event"].toObject()); break;
	case DiscoverySummary: processDiscoverySummarySignal(signalEvent); break;
	}
}

void Status::emitMessageSignal(QJsonObject ev)
{
	emit instance()->message(ev);
}

void Status::signalCallback(const char* data)
{
	QtConcurrent::run(instance(), &Status::processSignal, QString(data));
}

void Status::closeSession()
{
	QtConcurrent::run([=] {
		Logout();
		emit logout();
	});
}

QVariant Status::callPrivateRPC(QString method, QVariantList params)
{
	qDebug() << method;
	QJsonObject payload{{"jsonrpc", "2.0"}, {"method", method}, {"params", QJsonValue::fromVariant(params)}};
	QString payloadStr = Utils::jsonToStr(payload);

	// WARNING: uint64 are expected instead of strings.
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastClockValue\":\\s\"(\\d+?)\"")), QStringLiteral("\"lastClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"timestamp\":\\s\"(\\d+?)\"")), QStringLiteral("\"timestamp\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"deletedAtClockValue\":\\s\"(\\d+?)\"")), QStringLiteral("\"deletedAtClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"clock\":\\s\"(\\d+?)\"")), QStringLiteral("\"clock\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"clockValue\":\\s\"(\\d+?)\"")), QStringLiteral("\"clockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"whisperTimestamp\":\\s\"(\\d+?)\"")), QStringLiteral("\"whisperTimestamp\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"ensVerifiedAt\":\\s\"(\\d+?)\"")), QStringLiteral("\"ensVerifiedAt\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastENSClockValue\":\\s\"(\\d+?)\"")), QStringLiteral("\"lastENSClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastUpdated\":\\s\"(\\d+?)\"")), QStringLiteral("\"lastUpdated\":\\1"));

	const char* result = CallPrivateRPC(payloadStr.toUtf8().data());

	// WARNING: Signals are returning bigints as numeric values instead of strings
	QString r = QString(result);
	uint64ToStrReplacements(r);

	return QJsonDocument::fromJson(r.toUtf8()).toVariant();
}

void Status::callPrivateRPC(QString method, QVariantList params, const QJSValue& callback)
{
	auto* watcher = new QFutureWatcher<QVariant>(this);
	QObject::connect(watcher, &QFutureWatcher<QVariant>::finished, this, [this, watcher, callback]() {
		QVariant result = watcher->result();
		QJSValue cbCopy(callback); // needed as callback is captured as const
		QJSEngine* engine = qjsEngine(this);
		cbCopy.call(QJSValueList{engine->toScriptValue(result)});
		watcher->deleteLater();
	});
	watcher->setFuture(QtConcurrent::run(this, &Status::callPrivateRPC, method, params));
}

QString Status::getNodeVersion()
{
	const auto response = Status::instance()->callPrivateRPC("web3_clientVersion", QJsonArray{}.toVariantList()).toJsonObject();
	return QString(response["result"].toString());
}

bool Status::isOnline()
{
	return m_online;
}

QString Status::settingsPath()
{
	return Constants::applicationPath("settings/" + Settings::instance()->keyUID());
}
