#pragma once

#include <QObject>
#include <QString>
#include <QSet>
#include <QVector>
#include <QJsonObject>
#include <QQmlHelpers>
#include <QVariant>
#include "chat-type.hpp"
#include "content-type.hpp"
#include "message-type.hpp"
#include <QDebug>

class Message : public QObject
{
Q_OBJECT

public:
    explicit Message(QObject * parent = 0);
    explicit Message(const QJsonValue data, QObject * parent);

    QML_READONLY_PROPERTY(QString, id)
    QML_READONLY_PROPERTY(QString, alias)
    QML_READONLY_PROPERTY(QString, chatId)
    QML_READONLY_PROPERTY(QString, clock)
    // commandParameters: null
    QML_READONLY_PROPERTY(ContentType, contentType)
    QML_READONLY_PROPERTY(QString, ensName)
    QML_READONLY_PROPERTY(QString, from)
    QML_READONLY_PROPERTY(QString, identicon)
    QML_READONLY_PROPERTY(int, lineCount)
    QML_READONLY_PROPERTY(QString, localChatId)
    QML_READONLY_PROPERTY(MessageType, messageType)
    QML_READONLY_PROPERTY(bool, isNew)
    // parsedText
    //QML_READONLY_PROPERTY(QString, quotedMessage)
    //QML_READONLY_PROPERTY(QString, replace)
    QML_READONLY_PROPERTY(bool, rtl)
    QML_READONLY_PROPERTY(bool, seen)
    // sticker
    QML_READONLY_PROPERTY(QString, text)
    QML_READONLY_PROPERTY(QString, timestamp)
    QML_READONLY_PROPERTY(QString, whisperTimestamp)
};
