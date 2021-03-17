#pragma once

#include <QHash>
#include <QObject>
#include <QQmlHelpers>
#include <QReadWriteLock>
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
	Q_INVOKABLE void requestMessages(QString chatId, bool isOneToOne, int earliestKnownMessageTimestamp);
	Q_INVOKABLE void requestMessagesInLast(QString chatId, bool isOneToOne, int fetchRange);

	QString getActiveMailserver() const;

	Q_INVOKABLE void timeoutConnection(QString enode);
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
	mutable QReadWriteLock lock;

	QString generateSymKeyFromPassword();

	void requestMessagesCall(
		QVector<QString> topics, QString symKeyID, QString peer, int numberOfMessages, qint64 fromTimestamp, qint64 toTimestamp, bool force);

	QVector<Topic> getMailserverTopics();

	std::optional<QVector<Topic>> getMailserverTopicByChatId(QString chatId, bool isOneToOne);

signals:
	void cycle();
	void loopStopped();
	void mailserverAvailable();
	void requestSent();
	void checkTimeout(QString enode);

protected:
	void run() override;
};