#pragma once

#include "chat-type.hpp"
#include "contact.hpp"
#include "content-type.hpp"
#include "message-type.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

namespace Messages
{

struct Sticker
{
	QString hash;
	int pack;
};

class Message : public QObject
{
	Q_OBJECT

public:
	explicit Message(QObject* parent = nullptr);
	explicit Message(QString id, ContentType contentType, QObject* parent);
	explicit Message(const QJsonValue data, QObject* parent = nullptr);

	QML_READONLY_PROPERTY(QString, id)
	QML_READONLY_PROPERTY(QString, alias)
	QML_READONLY_PROPERTY(QString, chatId)
	QML_READONLY_PROPERTY(QString, clock)
	// commandParameters: null
	QML_READONLY_PROPERTY(QString, outgoingStatus);
	QML_READONLY_PROPERTY(ContentType, contentType)
	QML_READONLY_PROPERTY(QString, ensName)
	QML_READONLY_PROPERTY(QString, from)
	QML_READONLY_PROPERTY(QString, identicon)
	QML_READONLY_PROPERTY(int, lineCount)
	QML_READONLY_PROPERTY(QString, localChatId)
	QML_READONLY_PROPERTY(MessageType, messageType)
	QML_READONLY_PROPERTY(bool, isNew)
	QML_READONLY_PROPERTY(QJsonArray, parsedText)
	QML_READONLY_PROPERTY(QString, responseTo)
	//QML_READONLY_PROPERTY(QString, quotedMessage)
	QML_READONLY_PROPERTY(QString, replace)
	QML_READONLY_PROPERTY(bool, rtl)
	QML_READONLY_PROPERTY(bool, seen)
	QML_READONLY_PROPERTY(QString, text)
	QML_READONLY_PROPERTY(QString, timestamp)
	QML_READONLY_PROPERTY(QString, whisperTimestamp)
	QML_READONLY_PROPERTY(QString, linksUrls)
	QML_READONLY_PROPERTY(QString, image)
	QML_READONLY_PROPERTY(bool, hasMention)

	QML_READONLY_PROPERTY(Contact*, contact)

public:
	QString get_sticker_hash();

private:
	Sticker m_sticker;
};
} // namespace Messages