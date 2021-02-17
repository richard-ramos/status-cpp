
#include "status.hpp"
#include "QrCode.hpp"
#include "constants.hpp"
#include "libstatus.h"
#include "utils.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QJSEngine>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextDocumentFragment>
#include <QtConcurrent/QtConcurrent>

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
	SetSignalEventCallback((void*)&Status::statusGoEventCallback);

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
	input.replace(QRegularExpression(QStringLiteral("\"lastClockValue\":(\\d+)")),
				  QStringLiteral("\"lastClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"timestamp\":(\\d+)")),
				  QStringLiteral("\"timestamp\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"deletedAtClockValue\":(\\d+)")),
				  QStringLiteral("\"deletedAtClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"clock\":(\\d+)")),
				  QStringLiteral("\"clock\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"whisperTimestamp\":(\\d+)")),
				  QStringLiteral("\"whisperTimestamp\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"ensVerifiedAt\":(\\d+)")),
				  QStringLiteral("\"ensVerifiedAt\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"lastENSClockValue\":(\\d+)")),
				  QStringLiteral("\"lastENSClockValue\":\"\\1\""));
	input.replace(QRegularExpression(QStringLiteral("\"lastUpdated\":(\\d+)")),
				  QStringLiteral("\"lastUpdated\":\"\\1\""));
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

	qDebug() << "Signal received: " << signalType;
	
	switch(signalType)
	{
	case NodeLogin:
		emit instance()->login(signalEvent["event"]["error"].toString());
		break;
	case NodeReady:
		emit instance()->nodeReady(signalEvent["event"]["error"].toString());
		break;
	case NodeStopped:
		emit instance()->nodeStopped(signalEvent["event"]["error"].toString());
		break;
	case Message:
		emit instance()->message(signalEvent["event"].toObject());
		break;

	}
}

void Status::statusGoEventCallback(const char* event)
{
	QtConcurrent::run(instance(), &Status::processSignal, QString(event));
}

void Status::closeSession()
{
	QtConcurrent::run([=] {
		Logout();
		emit logout();
	});
}

QString Status::generateAlias(QString publicKey)
{
	return Utils::generateAlias(publicKey);
}

QString Status::generateIdenticon(QString publicKey)
{
	return Utils::generateIdenticon(publicKey);
}

QString Status::generateQRCode(QString publicKey)
{
	using namespace qrcodegen;
	// Create the QR Code object
	QStringList svg;
	QrCode qr = QrCode::encodeText(publicKey.toUtf8().data(), QrCode::Ecc::MEDIUM);
	qint32 sz = qr.getSize();
	int border = 2;
	svg << QString("data:image/svg+xml;utf8,");
	svg << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE svg PUBLIC \"-//W3C//DTD "
				   "SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");
	svg << QString(
			   "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %1 %2\" "
			   "stroke=\"none\"><rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/><path d=\"")
			   .arg(sz + border * 2)
			   .arg(sz + border * 2);
	for(int y = 0; y < sz; y++)
		for(int x = 0; x < sz; x++)
			if(qr.getModule(x, y))
				svg << QString("M%1,%2h1v1h-1z").arg(x + border).arg(y + border);

	svg << QString("\" fill=\"#000000\"/></svg>");

	return svg.join("");
}

QVariant Status::callPrivateRPC(QString method, QVariantList params)
{
	qDebug() << "CallPrivateRPC - method:" << method;

	QJsonObject payload{
		{"jsonrpc", "2.0"}, {"method", method}, {"params", QJsonValue::fromVariant(params)}};
	qDebug() << payload;
	QString payloadStr = Utils::jsonToStr(payload);

	// WARNING: uint64 are expected instead of strings.
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastClockValue\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"lastClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"timestamp\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"timestamp\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"deletedAtClockValue\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"deletedAtClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"clock\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"clock\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"whisperTimestamp\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"whisperTimestamp\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"ensVerifiedAt\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"ensVerifiedAt\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastENSClockValue\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"lastENSClockValue\":\\1"));
	payloadStr.replace(QRegularExpression(QStringLiteral("\"lastUpdated\":\\s\"(\\d+?)\"")),
					   QStringLiteral("\"lastUpdated\":\\1"));

	const char* result = CallPrivateRPC(payloadStr.toUtf8().data());

	// WARNING: Signals are returning bigints as numeric values instead of strings
	QString r = QString(result);
	uint64ToStrReplacements(r);

	return QJsonDocument::fromJson(r.toUtf8()).toVariant();
}

void Status::callPrivateRPC(QString method, QVariantList params, const QJSValue& callback)
{
	auto* watcher = new QFutureWatcher<QVariant>(this);
	QObject::connect(
		watcher, &QFutureWatcher<QVariant>::finished, this, [this, watcher, callback]() {
			QVariant result = watcher->result();
			QJSValue cbCopy(callback); // needed as callback is captured as const
			QJSEngine* engine = qjsEngine(this);
			cbCopy.call(QJSValueList{engine->toScriptValue(result)});
			watcher->deleteLater();
		});
	watcher->setFuture(QtConcurrent::run(this, &Status::callPrivateRPC, method, params));
}

void Status::copyToClipboard(const QString& value)
{
	return Utils::copyToClipboard(value);
}

QString Status::plainText(const QString& value)
{
	return QTextDocumentFragment::fromHtml( value ).toPlainText();
}