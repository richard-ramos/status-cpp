#include "messages-model.hpp"
#include "chat-type.hpp"
#include "contacts-model.hpp"
#include "content-type.hpp"
#include "message-format.hpp"
#include "message.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QRandomGenerator>
#include <QString>
#include <QStringBuilder>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>
#include <algorithm>
#include <array>

using namespace Messages;

MessagesModel::MessagesModel(QString chatId, ChatType chatType, QObject* parent)
	: m_chatId(chatId)
	, m_chatType(chatType)
	, QAbstractListModel(parent)
{
	qDebug() << "Creating MessageModel for chatId: " << m_chatId;
	QObject::connect(this, &MessagesModel::messageLoaded, this, QOverload<Message*>::of(&MessagesModel::push));
	QObject::connect(this, &MessagesModel::reactionLoaded, this, QOverload<QString, QJsonObject>::of(&MessagesModel::push));
	QObject::connect(Status::instance(), &Status::updateOutgoingStatus, this, &MessagesModel::updateOutgoingStatus);
	addFakeMessages();
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "messageId";
	roles[PlainText] = "plainText";
	roles[Contact] = "contact";
	roles[ContentType] = "contentType";
	roles[Clock] = "clock";
	roles[ChatId] = "chatId";
	roles[SectionIdentifier] = "sectionIdentifier";
	roles[Timestamp] = "timestamp";
	roles[ParsedText] = "parsedText";
	roles[Sticker] = "sticker";
	roles[ResponseTo] = "responseTo";
	roles[LinkUrls] = "linkUrls";
	roles[OutgoingStatus] = "outgoingStatus";
	roles[Image] = "image";
	roles[EmojiReactions] = "emojiReactions";
	roles[HasMention] = "hasMention";
	return roles;
}

int MessagesModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_messages.size();
}

QString sectionIdentifier(const Message* msg)
{
	if(msg->get_contentType() == ContentType::Group)
	{
		// Force section change, because group status messages are sent with the
		// same fromAuthor, and ends up causing the header to not be shown
		return "GroupChatMessage";
	}
	else
	{
		return msg->get_from();
	}
}

QVariant MessagesModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Message* msg = m_messages[index.row()];

	switch(role)
	{
	case Id: return QVariant(msg->get_id());
	case ResponseTo: return QVariant(msg->get_responseTo());
	case PlainText: return QVariant(msg->get_text());
	case Contact: return QVariant(msg->get_contentType() != ContentType::ChatIdentifier ? QVariant::fromValue(m_contacts->upsert(msg)) : "");
	case ContentType: return QVariant(msg->get_contentType());
	case Clock: return QVariant(msg->get_clock());
	case ChatId: return QVariant(msg->get_chatId());
	case Timestamp: return QVariant(msg->get_timestamp());
	case SectionIdentifier: return QVariant(sectionIdentifier(msg));
	case ParsedText: return QVariant(Messages::Format::renderBlock(msg, m_contacts));
	case Sticker: return QVariant(Messages::Format::decodeSticker(msg));
	case LinkUrls: return QVariant(Messages::Format::linkUrls(msg));
	case OutgoingStatus: return QVariant(msg->get_outgoingStatus());
	case Image: return QVariant(msg->get_image());
	case HasMention: return QVariant(msg->get_hasMention());
	case EmojiReactions: {
		QJsonArray emojiReactions = m_emojiReactions[msg->get_id()];
		return m_emojiReactions.contains(msg->get_id()) ? QVariant(Utils::jsonToStr(emojiReactions)) : QVariant("[]");
	}
	}

	return QVariant();
}

Message* MessagesModel::get(QString messageId) const
{
	return m_messageMap[messageId];
}

Message* MessagesModel::get(int row) const
{
	return m_messages[row];
}

void MessagesModel::push(Message* msg)
{
	if(msg->get_replace() != "")
	{
		// Delete existing message from UI since it's going to be replaced
		if(m_messageMap.contains(msg->get_id()))
		{
			int row = m_messages.indexOf(m_messageMap[msg->get_id()]);
			beginRemoveRows(QModelIndex(), row, row);
			QString id = msg->get_id();
			delete m_messageMap[id];
			m_messageMap.remove(id);
			m_messages.remove(row);
			endRemoveRows();
		}
	}

	if(m_messageMap.contains(msg->get_id())) return;

	m_contacts->upsert(msg);

	QQmlApplicationEngine::setObjectOwnership(msg, QQmlApplicationEngine::CppOwnership);
	msg->setParent(this);
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_messageMap[msg->get_id()] = msg;
	m_messages << msg;
	endInsertRows();

	emit newMessagePushed();
}

void MessagesModel::push(QString messageId, QJsonObject newReaction)
{
	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);
		if(m_emojiReactions.contains(messageId))
		{
			bool found = false;
			uint i = -1;
			bool retraction = false;
			foreach(const QJsonValue& oldReaction, m_emojiReactions[messageId])
			{
				i++;
				QJsonObject oldReactionObj = oldReaction.toObject();
				if(oldReactionObj["id"].toString() == newReaction["id"].toString())
				{
					found = true;
					if(newReaction["retracted"].toBool() == true)
					{
						retraction = true;
					}
					break;
				}
			}
			if(!found)
			{
				if(newReaction["retracted"].toBool()) return;
				m_emojiReactions[messageId] << newReaction;
			}
			else if(retraction)
			{
				m_emojiReactions[messageId].removeAt(i);
			}
		}
		else
		{
			m_emojiReactions[messageId] << newReaction;
		}
	});
}

void MessagesModel::loadMessages(bool initialLoad)
{
	if(!initialLoad && m_cursor == "") return;

	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);

		const auto response =
			Status::instance()->callPrivateRPC("wakuext_chatMessages", QJsonArray{m_chatId, m_cursor, 20}.toVariantList()).toJsonObject();
		m_cursor = response["result"]["cursor"].toString();

		// TODO: handle cursor
		foreach(QJsonValue msgJson, response["result"]["messages"].toArray())
		{
			Message* message = new Message(msgJson);
			message->moveToThread(QApplication::instance()->thread());

			if(m_chatType == ChatType::Timeline || m_chatType == ChatType::Profile)
			{
				emit statusUpdateLoaded(message);
			}
			else
			{
				emit messageLoaded(message);
			}
		}
	});
}

void MessagesModel::loadReactions(bool initialLoad)
{
	if(!initialLoad && m_reactionsCursor == "") return;

	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);

		const auto response = Status::instance()
								  ->callPrivateRPC("wakuext_emojiReactionsByChatID", QJsonArray{m_chatId, m_reactionsCursor, 20}.toVariantList())
								  .toJsonObject();
		m_reactionsCursor = response["result"]["cursor"].toString();

		QJsonArray reactions = response["result"].toArray();
		if(reactions.count() > 0)
		{
			foreach(const QJsonValue& reaction, reactions)
			{
				const QJsonObject r = reaction.toObject();
				emit reactionLoaded(r["messageId"].toString(), r);
			}
		}
	});
}

void MessagesModel::toggleReaction(QString messageId, int emojiId)
{
	if(!m_emojiReactions.contains(messageId))
	{
		const auto response =
			Status::instance()->callPrivateRPC("wakuext_sendEmojiReaction", QJsonArray{m_chatId, messageId, emojiId}.toVariantList()).toJsonObject();
		Status::instance()->emitMessageSignal(response["result"].toObject());
		return;
	}

	bool exists = false;
	int i = -1;
	QString reactionId("");
	foreach(const QJsonValue& reaction, m_emojiReactions[messageId])
	{
		i++;
		const QJsonObject r = reaction.toObject();
		if(r["emojiId"].toInt() == emojiId && r["from"] == Settings::instance()->publicKey())
		{
			exists = true;
			reactionId = r["id"].toString();
			break;
		}
	}
	if(exists)
	{
		m_emojiReactions[messageId].removeAt(i);
		const auto response =
			Status::instance()->callPrivateRPC("wakuext_sendEmojiReactionRetraction", QJsonArray{reactionId}.toVariantList()).toJsonObject();
		Status::instance()->emitMessageSignal(response["result"].toObject());
	}
	else
	{
		const auto response =
			Status::instance()->callPrivateRPC("wakuext_sendEmojiReaction", QJsonArray{m_chatId, messageId, emojiId}.toVariantList()).toJsonObject();
		Status::instance()->emitMessageSignal(response["result"].toObject());
	}
}

void MessagesModel::addFakeMessages()
{
	if(m_chatType != ChatType::Profile && m_chatType != ChatType::Timeline)
	{
		Message* fetchMoreMessages = new Message("fetchMoreMessages", ContentType::FetchMoreMessagesButton, this);
		QQmlApplicationEngine::setObjectOwnership(fetchMoreMessages, QQmlApplicationEngine::CppOwnership);

		Message* chatIdentifier = new Message("chatIdentifier", ContentType::ChatIdentifier, this);
		QQmlApplicationEngine::setObjectOwnership(chatIdentifier, QQmlApplicationEngine::CppOwnership);

		beginInsertRows(QModelIndex(), rowCount(), rowCount());
		m_messages << fetchMoreMessages;
		m_messages << chatIdentifier;
		endInsertRows();
	}
}

void MessagesModel::clear()
{
	beginResetModel();
	m_messages.clear();
	addFakeMessages();
	endResetModel();
}

void MessagesModel::updateOutgoingStatus(QVector<QString> messageIds, bool sent)
{
	foreach(const QString& messageId, messageIds)
	{
		if(!m_messageMap.contains(messageId)) continue;
		m_messageMap[messageId]->update_outgoingStatus(sent ? "sent" : "not-sent");
		Status::instance()
			->callPrivateRPC("wakuext_updateMessageOutgoingStatus",
							 QJsonArray{messageId, m_messageMap[messageId]->get_outgoingStatus()}.toVariantList())
			.toJsonObject();
		int index = m_messages.indexOf(m_messageMap[messageId]);
		QModelIndex idx = createIndex(index, 0);
		dataChanged(idx, idx);
	}
}

void MessagesModel::resend(QString messageId)
{
	if(!m_messageMap.contains(messageId)) return;

	m_messageMap[messageId]->update_outgoingStatus("sending");
	Status::instance()->callPrivateRPC("wakuext_updateMessageOutgoingStatus", QJsonArray{messageId, QStringLiteral("sending")}.toVariantList());
	Status::instance()->callPrivateRPC("wakuext_reSendChatMessage", QJsonArray{messageId}.toVariantList());

	int index = m_messages.indexOf(m_messageMap[messageId]);
	QModelIndex idx = createIndex(index, 0);
	dataChanged(idx, idx);
}

void MessagesModel::removeFrom(QString contactId)
{
	foreach(Message* message, m_messages)
	{
		if(message->get_from() != contactId) continue;
		QString id = message->get_id();
		int index = m_messages.indexOf(m_messageMap[id]);
		if(index == -1) continue;

		beginRemoveRows(QModelIndex(), index, index);
		delete m_messageMap[id];
		m_messageMap.remove(id);
		m_messages.remove(index);
		endRemoveRows();
	}
}