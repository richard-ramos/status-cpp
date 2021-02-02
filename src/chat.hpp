#pragma once

#include "chat-type.hpp"
#include "messages-model.hpp"
#include <QDebug>
#include <QJsonObject>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

class Chat : public QObject
{
	Q_OBJECT

public:
	explicit Chat(QString id,
				  ChatType chatType,
				  QObject* parent = 0,
				  QString name = "",
				  QString profile = "",
				  QString color = "",
				  bool active = true,
				  QString timestamp = "0",
				  QString lastClockValue = "0",
				  QString deletedAtClockValue = "0",
				  int unviewedMessagesCount = 0,
				  bool muted = false); //, QJsonValue jsonChat);
	explicit Chat(const QJsonValue data, QObject* parent);

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
	// QML_READONLY_PROPERTY(Message, lastMessage)
	//QSet<ChatMember> m_members;
	//QVector<ChatMembershipEvent> m_membershipUpdateEvents;
	QML_READONLY_PROPERTY(bool, muted)

	// hasMentions
	// ensName

	QML_READONLY_PROPERTY(MessagesModel*, messages)

public:
	Q_INVOKABLE void save();
	Q_INVOKABLE void sendMessage(QString message, bool isReply, bool isEmoji);
	void update(const QJsonValue data);
	void loadFilter();
};
