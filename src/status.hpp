#pragma once

#include <QJSValue>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>

class Status : public QObject
{
	Q_OBJECT

public:
	~Status() { }

	static Status* instance();

	enum SignalType
	{
		Unknown,
		Message,
		EnvelopeSent,
		EnvelopeExpired,
		WhisperFilterAdded,
		Wallet,
		NodeLogin,
		NodeReady,
		NodeStarted,
		NodeStopped,
		MailserverRequestCompleted,
		MailserverRequestExpired,
		DiscoveryStarted,
		DiscoveryStopped,
		DiscoverySummary,
		SubscriptionsData,
		SubscriptionsError
	};

	Q_ENUM(SignalType)

	Q_INVOKABLE QString generateAlias(QString publicKey);
	Q_INVOKABLE QString generateIdenticon(QString publicKey);
	Q_INVOKABLE QString generateQRCode(QString publicKey);

	Q_INVOKABLE void copyToClipboard(const QString& value); // TODO: extract to utils singleton
	Q_INVOKABLE QString plainText(const QString& value);

	Q_INVOKABLE QVariant callPrivateRPC(QString method, QVariantList params);
	Q_INVOKABLE void callPrivateRPC(QString method, QVariantList params, const QJSValue& callback);
	Q_INVOKABLE void closeSession();

signals:
	void signal(SignalType signal);
	void login(QString error);
	void nodeReady(QString error);
	void nodeStopped(QString error);
	void message(QJsonObject update);
	void logout();

private:
	static Status* theInstance;
	explicit Status(QObject* parent = nullptr);
	static std::map<QString, SignalType> signalMap;
	static void statusGoEventCallback(const char* event);
	void processSignal(QString ev);
};
