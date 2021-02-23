#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QString>
#include <QThread>
#include <QVector>

class MailserverCycle : public QThread
{
	Q_OBJECT

	QML_READONLY_PROPERTY(QString, activeMailserver)

public:
	MailserverCycle(QObject* parent = nullptr);
	~MailserverCycle();

	Q_INVOKABLE void work();
	Q_INVOKABLE void peerSummaryChange(QVector<QString> peers);

	void timeoutConnection(QString enode);

	enum MailserverStatus
	{
		Disconnected = 0,
		Connecting = 1,
		Connected = 2,
		Trusted = 3
	};

private:
	void findNewMailserver();
	void disconnectActiveMailserver();
	void connect(QString enode);
	void trustPeer(QString enode);
	void updateMailserver(QString enode);

	QHash<QString, MailserverStatus> nodes;

	QVector<QString> getMailservers();
	QMutex m_mutex;

signals:
	void cycle();
	void loopStopped();
	void mailserverAvailable();

protected:
	void run() override;
};