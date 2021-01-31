#ifndef CHAT_H
#define CHAT_H

#include <QObject>
#include <QString>
#include <QSet>
#include <QVector>
#include <QJsonObject>
#include <QQmlHelpers>
#include <QVariant>
#include "chat-type.hpp"

class Chat : public QObject
{
Q_OBJECT

public:
    explicit Chat(QString id, ChatType chatType, QString name = "", QString profile = "", QString color = "", bool active = true, QString timestamp = "0", QString lastClockValue = "0", QString deletedAtClockValue = "0", int unviewedMessagesCount = 0, bool muted = false, QObject * parent = 0); //, QJsonValue jsonChat);
    explicit Chat(const QJsonValue data, QObject * parent);

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
public:
    //Q_INVOKABLE update(QJsonValue jsonChat);
    Q_INVOKABLE void save();
    Q_INVOKABLE void sendMessage(QString message, bool isReply, bool isEmoji);
};

#endif // CHAT_H