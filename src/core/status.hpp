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


	// TODO: extract to utils singleton
	Q_INVOKABLE void copyToClipboard(const QString& value); 
	Q_INVOKABLE QString plainText(const QString& value);
	Q_INVOKABLE QString getNodeVersion();
	Q_PROPERTY(QString NodeVersion READ getNodeVersion CONSTANT)



	Q_INVOKABLE QVariant callPrivateRPC(QString method, QVariantList params);
	Q_INVOKABLE void callPrivateRPC(QString method, QVariantList params, const QJSValue& callback);
	Q_INVOKABLE void closeSession();

	Q_PROPERTY(QString SettingsPath READ settingsPath CONSTANT)
	QString settingsPath();

	void emitMessageSignal(QJsonObject ev);

	Q_PROPERTY(bool IsOnline READ isOnline NOTIFY onlineStatusChanged)

signals:
	void signal(SignalType signal);
	void login(QString error);
	void nodeReady(QString error);
	void nodeStopped(QString error);
	void message(QJsonObject update);
	void discoverySummary(QVector<QString> enodes);
	void logout();

	void onlineStatusChanged(bool connected);

private:
	static Status* theInstance;
	explicit Status(QObject* parent = nullptr);
	static std::map<QString, SignalType> signalMap;
	static void signalCallback(const char* data);
	void processSignal(QString ev);
	void processDiscoverySummarySignal(const QJsonObject& signalEvent);

	bool isOnline();

	bool m_online;
};
