#pragma once

#include "chat-type.hpp"
#include "contact.hpp"
#include "mailserver-cycle.hpp"
#include "message.hpp"
#include "messages-model.hpp"
#include <QDebug>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

struct ChatMember
{
	bool admin;
	QString id;
	bool joined;

	Q_PROPERTY(bool admin MEMBER admin)
	Q_PROPERTY(QString id MEMBER id)
	Q_PROPERTY(bool joined MEMBER joined)

	bool operator==(const ChatMember& a) const
	{
		return (id == a.id);
	}

	Q_GADGET
};
Q_DECLARE_METATYPE(ChatMember)

struct ChatMembershipEvent
{
	QString chatId;
	QString clockValue;
	QString from;
	QString name;
	QString rawPayload;
	QString signature;
	int type;
};

class Chat : public QObject
{
	Q_OBJECT

public:
	explicit Chat(QObject* parent,
				  QString id,
				  ChatType chatType,
				  QString name = "",
				  QString profile = "",
				  QString color = "",
				  bool active = true,
				  QString timestamp = "0",
				  QString lastClockValue = "0",
				  QString deletedAtClockValue = "0",
				  int unviewedMessagesCount = 0,
				  bool muted = false);
	explicit Chat(QObject* parent, const QJsonValue data);
	virtual ~Chat();

	QML_READONLY_PROPERTY(QString, id)
	QML_READONLY_PROPERTY(QString, name)
	QML_READONLY_PROPERTY(QString, profile)
	//QML_READONLY_PROPERTY(QString, communityId)
	//QML_READONLY_PROPERTY(QString, description)
	QML_READONLY_PROPERTY(QString, color)
	QML_READONLY_PROPERTY(QString, identicon)
	QML_READONLY_PROPERTY(bool, active)
	QML_READONLY_PROPERTY(ChatType, chatType)
	QML_READONLY_PROPERTY(QString, timestamp)
	QML_READONLY_PROPERTY(QString, lastClockValue)
	QML_READONLY_PROPERTY(QString, deletedAtClockValue)
	QML_READONLY_PROPERTY(int, unviewedMessagesCount)
	QML_READONLY_PROPERTY(Message*, lastMessage)
	QML_READONLY_PROPERTY(bool, muted)
	QML_READONLY_PROPERTY(bool, hasMentions)

	// ensName

	QML_READONLY_PROPERTY(MessagesModel*, messages)
	QML_READONLY_PROPERTY(Contact*, contact)

	Q_PROPERTY(QSet<ChatMember> chatMembers READ getChatMembers NOTIFY groupDataChanged)

signals:
	void left(QString chatId);
	void sendingMessage();
	void sendingMessageFailed();
	void messagesLoaded();
	void topicCreated(Topic t);
	void groupDataChanged();

private:
	QMutex m_mutex;
	QSet<ChatMember> m_members;
	QVector<ChatMembershipEvent> m_membershipUpdateEvents;

public:
	Q_INVOKABLE void save();
	Q_INVOKABLE void sendMessage(QString message, QString replyTo, bool isEmoji);
	Q_INVOKABLE void sendSticker(int packId, QString stickerHash);
	Q_INVOKABLE void sendImage(QString imagePath);
	Q_INVOKABLE void leave();
	Q_INVOKABLE void loadMoreMessages();
	Q_INVOKABLE void deleteChatHistory();
	Q_INVOKABLE void markAllMessagesAsRead();
	Q_INVOKABLE void requestMoreMessages(qint64);
	Q_INVOKABLE void requestMessagesInLast(int fetchRange);

	Q_INVOKABLE void join();

	Q_INVOKABLE void renameGroup(QString newName);
	Q_INVOKABLE void makeAdmin(QString memberId);
	Q_INVOKABLE void removeFromGroup(QString memberId);
	Q_INVOKABLE void addMembers(QStringList members);

	bool operator==(const Chat& c);

	void update(const QJsonValue data);
	void loadFilter();
	void leaveGroup();

	QSet<ChatMember> getChatMembers();
};

uint qHash(const ChatMember& item, uint seed = 0);
