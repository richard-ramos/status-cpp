#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QString>
#include <QThread>
#include <QVector>

struct Topic
{
	QString topic;
	bool discovery;
	bool negotiated;
	QVector<QString> chatIds;
	int lastRequest;
};

class MailserverCycle : public QThread
{
	Q_OBJECT

	QML_READONLY_PROPERTY(QString, activeMailserver)

public:
	MailserverCycle(QObject* parent = nullptr);
	~MailserverCycle();

	Q_INVOKABLE void work();
	Q_INVOKABLE void peerSummaryChange(QVector<QString> peers);
	Q_INVOKABLE void addChannelTopic(Topic t);
	Q_INVOKABLE void initialMailserverRequest();


	void timeoutConnection(QString enode);
	void requestMessages(QVector<QString> topicList, qint64 fromValue = 0, qint64 toValue = 0, bool force = false);
	void removeMailserverTopicForChat(QString chatId);

	enum MailserverStatus
	{
		Disconnected = 0,
		Connecting = 1,
		Connected = 2,
	};

private:
	void findNewMailserver();
	void disconnectActiveMailserver();
	void connect(QString enode);
	void trustPeer(QString enode);
	void updateMailserver(QString enode);
	bool isMailserverAvailable();

	void addMailserverTopic(Topic t);

	QHash<QString, MailserverStatus> nodes;

	QVector<QString> getMailservers();
	QMutex m_mutex;

	QString generateSymKeyFromPassword();

	void requestMessagesCall(
		QVector<QString> topics, QString symKeyID, QString peer, int numberOfMessages, qint64 fromTimestamp, qint64 toTimestamp, bool force);

	QVector<Topic> getMailserverTopics();


signals:
	void cycle();
	void loopStopped();
	void mailserverAvailable();
	void requestSent();

protected:
	void run() override;
};