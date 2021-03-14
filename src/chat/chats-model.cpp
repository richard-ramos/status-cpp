#include "chats-model.hpp"
#include "chat-type.hpp"
#include "chat.hpp"
#include "constants.hpp"
#include "contact.hpp"
#include "contacts-model.hpp"
#include "mailserver-cycle.hpp"
#include "mailserver-model.hpp"
#include "message-format.hpp"
#include "message.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QVariantList>
#include <algorithm>
#include <array>

using namespace Messages;

ChatsModel::ChatsModel(QObject* parent)
	: QAbstractListModel(parent)
{
	QObject::connect(Status::instance(), &Status::message, this, &ChatsModel::update);
	QObject::connect(this, &ChatsModel::joined, this, &ChatsModel::added);
	QObject::connect(this, &ChatsModel::contactsChanged, this, &ChatsModel::onContactsChanged);
	QObject::connect(this, &ChatsModel::mailserversChanged, this, &ChatsModel::onMailserversChanged);
	init();
}

void ChatsModel::init()
{
	loadChats();
	addTimelineChat();
	startMessenger();
}

void ChatsModel::addTimelineChat()
{
	Chat* timelineChat = new Chat(this, Constants::getTimelineChatId(), ChatType::Timeline);
	timelineChat->save();
	m_chatMap[timelineChat->get_id()] = timelineChat;
}

void ChatsModel::onContactsChanged()
{
	QObject::connect(m_contacts, &ContactsModel::contactToggled, this, &ChatsModel::toggleTimelineChat);

	// Loading messages for timeline chats
	m_chatMap[Constants::getTimelineChatId()]->get_messages()->set_contacts(m_contacts);
	foreach(Chat* chat, m_timelineChats)
	{
		chat->get_messages()->set_contacts(m_contacts);
		chat->get_messages()->loadMessages();
		chat->get_messages()->loadReactions();
	}

	// Loading messages for standard chats
	foreach(Chat* chat, m_chats)
	{
		chat->get_messages()->set_contacts(m_contacts);
		chat->get_messages()->loadMessages();
		chat->get_messages()->loadReactions();

		m_contacts->upsert(chat);
	}
}

void ChatsModel::onMailserversChanged()
{
	foreach(Chat* chat, m_timelineChats)
	{
		chat->set_mailservers(m_mailservers);
	}

	foreach(Chat* chat, m_chats)
	{
		chat->set_mailservers(m_mailservers);
	}
}

QHash<int, QByteArray> ChatsModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "chatId";
	roles[Name] = "name";
	roles[Muted] = "muted";
	roles[Identicon] = "identicon";
	roles[UnreadMessages] = "unviewedMessagesCount";
	roles[HasMentions] = "hasMentions";
	roles[ContentType] = "contentType";
	roles[Type] = "chatType";
	roles[Color] = "color";
	roles[Timestamp] = "timestamp";
	roles[Messages] = "messages";
	roles[LastMessage] = "lastMessage";
	roles[Contact] = "contact";
	roles[ChatMembers] = "chatMembers";

	return roles;
}

int ChatsModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_chats.size();
}

QVariant ChatsModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Chat* chat = m_chats[index.row()];

	switch(role)
	{
	case Id: return QVariant(chat->get_id());
	case Name: return QVariant(chat->get_name());
	case Muted: return QVariant(chat->get_muted());
	case Identicon: return QVariant(chat->get_identicon());
	case UnreadMessages: return QVariant(chat->get_unviewedMessagesCount());
	case Type: return QVariant(chat->get_chatType());
	case Color: return QVariant(chat->get_color());
	case HasMentions: return QVariant(chat->get_hasMentions());
	case Timestamp: return QVariant(chat->get_timestamp());
	case LastMessage: return QVariant(Messages::Format::renderSimpleText(chat->get_lastMessage(), m_contacts));
	case Messages: return QVariant::fromValue(chat->get_messages());
	case Contact: return QVariant(chat->get_chatType() == ChatType::OneToOne ? QVariant::fromValue(m_contacts->upsert(chat)) : "");
	case ContentType: return QVariant(chat->get_lastMessage()->get_contentType());
	case ChatMembers: return QVariant(QVariant::fromValue(chat->getChatMembers()));
	}
	// TODO: case HasMentions: return QVariant(chat->get_hasMentions());
	//TODO: case ContentType: return QVariant(chat->get_contentType());
	return QVariant();
}

void ChatsModel::pushStatusUpdate(Message* msg)
{
	m_chatMap[Constants::getTimelineChatId()]->get_messages()->push(msg);
}

void ChatsModel::insert(Chat* chat)
{
	QQmlApplicationEngine::setObjectOwnership(chat, QQmlApplicationEngine::CppOwnership);
	chat->setParent(this);
	chat->get_messages()->set_contacts(m_contacts);
	chat->set_mailservers(m_mailservers);
	m_chatMap[chat->get_id()] = chat;

	if(chat->get_chatType() == ChatType::Profile || chat->get_chatType() == ChatType::Timeline)
	{
		// Status updates should not appear in channel list
		m_timelineChats << chat;
		// Loaded messages from contact updates will be pushed into timeline chat
		QObject::connect(chat->get_messages(), &MessagesModel::statusUpdateLoaded, this, &ChatsModel::pushStatusUpdate);
	}
	else
	{
		beginInsertRows(QModelIndex(), rowCount(), rowCount());
		m_chats << chat;
		endInsertRows();
	}
}

void ChatsModel::join(ChatType chatType, QString id, QString ensName)
{
	if(!m_chatMap.contains(id))
	{
		qDebug() << "Chat does not exist. Creating chat: " << id;
		try
		{
			Chat* c = new Chat(this, id, chatType, ensName);
			c->save();

			m_contacts->upsert(c);

			QObject::connect(c, &Chat::topicCreated, m_mailservers->getCycle(), &MailserverCycle::addChannelTopic);

			insert(c);

			c->loadFilter();
			emit joined(chatType, id, m_chats.count() - 1);
		}
		catch(const std::exception& e)
		{
			qWarning() << "Error saving chat: " << e.what();
			emit joinError(e.what());
		}
		qDebug() << "Chat saved";
	}
	else
	{
		// Channel already joined
		int chatIndex = m_chats.indexOf(m_chatMap[id]);
		emit joined(chatType, id, chatIndex);
	}
}

void ChatsModel::createGroup(QString groupName, QStringList members)
{
	const auto response = Status::instance()
							  ->callPrivateRPC("wakuext_createGroupChatWithMembers",
											   QJsonArray{QJsonValue(), groupName, QJsonArray::fromStringList(members)}.toVariantList())
							  .toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());

	// TO
	emit joined(ChatType::PrivateGroupChat, m_chats[m_chats.count() - 1]->get_id(), m_chats.count() - 1);
}

void ChatsModel::startMessenger()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_startMessenger", QJsonArray{}.toVariantList()).toJsonObject();
	// TODO: do something with mailservers/ranges?
}

void ChatsModel::loadChats()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_chats", QJsonArray{}.toVariantList()).toJsonObject();

	if(response["result"].isNull()) return;

	foreach(const QJsonValue& value, response["result"].toArray())
	{
		const QJsonObject obj = value.toObject();
		if(!value["active"].toBool()) continue;
		Chat* c = new Chat(this, obj);
		insert(c);
		emit added(c->get_chatType(), c->get_id(), m_chats.count() - 1);
	}
}

Chat* ChatsModel::get(int row) const
{
	if(row < 0) return nullptr;
	return m_chats[row];
}

void ChatsModel::removeFilterRPC(QString chatId, QString filterId)
{
	QJsonObject obj{{"ChatID", chatId}, {"FilterID", filterId}};
	const auto response = Status::instance()->callPrivateRPC("wakuext_removeFilters", QJsonArray{QJsonArray{obj}}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw std::domain_error(response["error"]["message"].toString().toUtf8());
	}
}

void ChatsModel::remove1on1Filters(QString chatId, QJsonArray filters)
{
	QString partitionedTopic;
	foreach(const QJsonValue& filterJson, filters)
	{
		const QJsonObject filter = filterJson.toObject();

		// Contact code filter should be removed
		if(filter["identity"].toString() == chatId && filter["chatId"].toString().endsWith("-contact-code"))
		{
			removeFilterRPC(chatId, filter["chatId"].toString());
		}

		// Remove partitioned topic if no other user in an active group chat or one-to-one is from the
		// same partitioned topic
		if(filter["identity"].toString() == chatId && filter["chatId"].toString().startsWith("contact-discovery-"))
		{
			partitionedTopic = filter["topic"].toString();
			bool samePartitionedTopic = false;
			foreach(const QJsonValue& fJson, filters)
			{
				const QJsonObject f = fJson.toObject();
				if(f["topic"].toString() == partitionedTopic && f["filterId"].toString() != filter["filterId"].toString())
				{
					QString fIdentity = f["identity"].toString();
					if(m_chatMap.contains(fIdentity) && m_chatMap[fIdentity]->get_active())
					{
						samePartitionedTopic = true;
						break;
					}
				}
			}

			if(!samePartitionedTopic)
			{
				removeFilterRPC(chatId, filter["filterId"].toString());
			}
		}
	}
}

void ChatsModel::removeFilter(Chat* c)
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_filters", QJsonArray{}.toVariantList()).toJsonObject();
	QJsonArray filters = response["result"].toArray();

	switch(c->get_chatType())
	{
	case ChatType::Profile:
	case ChatType::Public: {
		foreach(const QJsonValue& filterJson, filters)
		{
			const QJsonObject filter = filterJson.toObject();
			if(filter["chatId"].toString() == c->get_id())
			{
				removeFilterRPC(c->get_id(), filter["filterId"].toString());
			}
		}
	}
	break;
	case ChatType::OneToOne: {
		// Check if user does not belong to any active chat group
		bool inGroup = false;
		ChatMember member;
		member.id = c->get_id();
		foreach(Chat* chat, m_chats)
		{
			if(chat->get_active() && chat->get_chatType() == ChatType::PrivateGroupChat && chat->getChatMembers().contains(member))
			{
				inGroup = true;
			}
		}

		if(!inGroup)
		{
			remove1on1Filters(c->get_id(), filters);
		}
	}
	break;
	case ChatType::PrivateGroupChat: {
		foreach(const ChatMember& member, c->getChatMembers())
		{
			// Check that any of the members are not in other active group chats, or that you don’t have a one-to-one open.
			bool hasConversation = false;
			foreach(Chat* chat, m_chats)
			{
				if((chat->get_active() && chat->get_chatType() == ChatType::OneToOne && chat->get_id() == member.id) ||
				   (chat->get_active() && chat->get_id() != c->get_id() && chat->get_chatType() == ChatType::PrivateGroupChat &&
					chat->getChatMembers().contains(member)))
				{
					hasConversation = true;
					break;
				}
			}

			if(!hasConversation)
			{
				if(m_chatMap.contains(member.id))
				{
					remove1on1Filters(member.id, filters);
				}
			}
		}
	}
	break;
	default: qWarning() << "Unhandled chatType" << c->get_id() << c->get_chatType();
	}

	m_mailservers->getCycle()->removeMailserverTopicForChat(c->get_id());
}

void ChatsModel::remove(int row)
{
	removeFilter(m_chats[row]);

	m_chats[row]->leave();
	m_chatMap.remove(m_chats[row]->get_id());
	beginRemoveRows(QModelIndex(), row, row);
	delete m_chats[row];
	m_chats.remove(row);
	endRemoveRows();
	left(row);
}

void ChatsModel::markAllMessagesAsRead(int row)
{
	m_chats[row]->markAllMessagesAsRead();
	QModelIndex idx = createIndex(row, 0);
	dataChanged(idx, idx);
}

void ChatsModel::deleteChatHistory(int row)
{
	m_chats[row]->deleteChatHistory();
	QModelIndex idx = createIndex(row, 0);
	dataChanged(idx, idx);
}

void ChatsModel::update(QJsonValue updates)
{
	// Process chats
	foreach(QJsonValue chatJson, updates["chats"].toArray())
	{
		QString chatId = chatJson["id"].toString();
		if(m_chatMap.contains(chatId))
		{
			int chatIndex = m_chats.indexOf(m_chatMap[chatId]);
			m_chatMap[chatId]->update(chatJson);
			if(chatIndex > -1)
			{
				QModelIndex idx = createIndex(chatIndex, 0);
				dataChanged(idx, idx);
			}
		}
		else
		{
			Chat* newChat = new Chat(this, chatJson);
			m_contacts->upsert(newChat);
			insert(newChat);
			emit added(newChat->get_chatType(), newChat->get_id(), m_chats.count() - 1);
		}
		// TODO: tell @cammellos that the messages are not returning the ens name
		m_contacts->upsert(m_chatMap[chatId]->get_lastMessage());
		m_contacts->upsert(m_chatMap[chatId]);
	}

	// Messages
	foreach(QJsonValue msgJson, updates["messages"].toArray())
	{
		Message* message = new Message(msgJson);

		QString chatId = Constants::getTimelineChatId(message->get_from()) == message->get_localChatId() ? Constants::getTimelineChatId()
																										 : message->get_localChatId();

		m_chatMap[chatId]->get_messages()->push(message);
		if(message->get_hasMention())
		{
			m_chatMap[chatId]->update_hasMentions(true);
			int chatIndex = m_chats.indexOf(m_chatMap[chatId]);
			if(chatIndex > -1)
			{
				QModelIndex idx = createIndex(chatIndex, 0);
				dataChanged(idx, idx);
			}
		}
		// Create a contact if necessary
		m_contacts->upsert(message);
	}

	// Emoji reactions
	if(!updates["emojiReactions"].isUndefined())
	{
		foreach(QJsonValue reaction, updates["emojiReactions"].toArray())
		{
			QJsonObject r = reaction.toObject();
			QString chatId = r["localChatId"].toString();
			QString messageId = r["messageId"].toString();
			m_chatMap[chatId]->get_messages()->push(messageId, r);
		}
	}
}

void ChatsModel::toggleTimelineChat(QString contactId, bool contactWasAdded)
{
	QString timelineChatId = Constants::getTimelineChatId(contactId);
	if(contactWasAdded)
	{
		if(m_chatMap.contains(timelineChatId)) return;

		Chat* c = new Chat(this, timelineChatId, ChatType::Profile, "", contactId);
		c->save();
		QObject::connect(c, &Chat::topicCreated, m_mailservers->getCycle(), &MailserverCycle::addChannelTopic);
		insert(c);
		c->loadFilter();
	}
	else
	{
		if(!m_chatMap.contains(timelineChatId)) return;

		removeTimelineMessages(contactId);

		removeFilter(m_chatMap[timelineChatId]);
		m_chatMap[timelineChatId]->leave();
		int index = m_timelineChats.indexOf(m_chatMap[timelineChatId]);
		m_chatMap.remove(timelineChatId);
		qDebug() << index << "CHAT INDEX!!!";
		delete m_timelineChats[index];
		m_timelineChats.remove(index);
	}
}

QVariant ChatsModel::timelineMessages()
{
	return QVariant::fromValue(m_chatMap[Constants::getTimelineChatId()]->get_messages());
}

void ChatsModel::removeTimelineMessages(QString contactId)
{
	m_chatMap[Constants::getTimelineChatId()]->get_messages()->removeFrom(contactId);
}
