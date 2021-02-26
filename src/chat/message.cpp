#include "message.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

using namespace Messages;

Message::Message(QObject* parent)
	: QObject(parent)
{ }

Message::Message(QString id, ContentType contentType, QObject* parent)
	: QObject(parent)
	, m_id(id)
	, m_contentType(contentType)
{ }

QString Message::get_sticker_hash()
{
	return m_sticker.hash;
}

Message::Message(const QJsonValue data, QObject* parent)
	: QObject(parent)
	, m_id(data["id"].toString())
	, m_alias(data["alias"].toString())
	, m_chatId(data["chatId"].toString())
	, m_clock(data["clock"].toString())
	, m_ensName(data["ensName"].toString())
	, m_from(data["from"].toString())
	, m_identicon(data["identicon"].toString())
	, m_lineCount(data["lineCount"].toInt())
	, m_localChatId(data["localChatId"].toString())
	, m_isNew(data["new"].toBool())
	, m_rtl(data["rtl"].toBool())
	, m_seen(data["seen"].toBool())
	, m_text(data["text"].toString())
	, m_timestamp(data["timestamp"].toString())
	, m_whisperTimestamp(data["whisperTimestamp"].toString())
	, m_parsedText(data["parsedText"].toArray())
	, m_responseTo(data["responseTo"].toString())
	, m_outgoingStatus(data["outgoingStatus"].toString())

{
	int contentType = data["contentType"].toInt();
	if(contentType < ContentType::FetchMoreMessagesButton || contentType > ContentType::Community)
	{
		m_contentType = ContentType::Unknown;
	}
	else
	{
		m_contentType = static_cast<ContentType>(contentType);
	}

	int messageType = data["messageType"].toInt();
	if(messageType < MessageType::Unknown || messageType > MessageType::CommunityChat)
	{
		m_messageType = MessageType::Unknown;
	}
	else
	{
		m_messageType = static_cast<MessageType>(messageType);
	}

	if(m_contentType == ContentType::Sticker)
	{
		m_sticker.hash = data["sticker"]["hash"].toString();
		m_sticker.pack = data["sticker"]["pack"].toInt();
	}
}
