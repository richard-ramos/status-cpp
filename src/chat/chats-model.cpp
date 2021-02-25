#include "chats-model.hpp"
#include "chat-type.hpp"
#include "chat.hpp"
#include "message-format.hpp"
#include "message.hpp"
#include "status.hpp"
#include "settings.hpp"
#include "utils.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <algorithm>
#include <array>
#include "mailserver-cycle.hpp"

using namespace Messages;

ChatsModel::ChatsModel(QObject* parent)
	: QAbstractListModel(parent)
{
	init();

	QObject::connect(Status::instance(), &Status::message, this, &ChatsModel::update);
	QObject::connect(this, &ChatsModel::joined, this, &ChatsModel::added);
	QObject::connect(this, &ChatsModel::contactsChanged, this, &ChatsModel::setupMessageModel);
}

void ChatsModel::init()
{
	loadChats();
	startMessenger();
}

void ChatsModel::setupMessageModel()
{
	foreach(Chat* chat, m_chats)
	{
		chat->get_messages()->set_contacts(m_contacts);
		chat->get_messages()->loadMessages();
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
	case Timestamp: return QVariant(chat->get_timestamp());
	case LastMessage: return QVariant(Messages::Format::renderSimpleText(chat->get_lastMessage(), m_contacts));
	case Messages: return QVariant(QVariant::fromValue(chat->get_messages()));
	}
	// TODO: case HasMentions: return QVariant(chat->get_hasMentions());
	//TODO: case ContentType: return QVariant(chat->get_contentType());
	return QVariant();
}

void ChatsModel::insert(Chat* chat)
{
	QQmlApplicationEngine::setObjectOwnership(chat, QQmlApplicationEngine::CppOwnership);
	chat->setParent(this);
	chat->get_messages()->set_contacts(m_contacts);
	m_chats << chat;
	m_chatMap[chat->get_id()] = chat;
}

void ChatsModel::join(ChatType chatType, QString id)
{
	if(!m_chatMap.contains(id))
	{
		qDebug() << "ChatsModel::join - Chat does not exist. Creating chat: " << id;
		try
		{
			Chat* c = new Chat(id, chatType);
			c->save();

			QObject::connect(c, &Chat::topicCreated, &Settings::instance()->mailserverCycle, &MailserverCycle::addChannelTopic);

			beginInsertRows(QModelIndex(), rowCount(), rowCount());
			insert(c);
			endInsertRows();
			c->loadFilter();
			emit joined(chatType, id, m_chats.count() - 1);
		}
		catch(const std::exception& e)
		{
			qWarning() << "ChatsModel::join - Error saving chat: " << e.what();
			emit joinError(e.what());
		}
		qDebug() << "ChatsModel::join - Chat saved";
	}
}

void ChatsModel::startMessenger()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_startMessenger", QJsonArray{}.toVariantList()).toJsonObject();
	// TODO: do something with mailservers/ranges

	foreach(const QJsonValue& filter, response["result"]["filters"].toArray())
	{
		// Add code for private chats
		QString chatId = filter["chatId"].toString();
		if(m_chatMap.contains(chatId))
		{
			m_chatMap[chatId]->setFilterId(filter["filterId"].toString());
		}
	}
}

void ChatsModel::loadChats()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_chats", QJsonArray{}.toVariantList()).toJsonObject();

	if(response["result"].isNull())
		return;

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	foreach(const QJsonValue& value, response["result"].toArray())
	{
		const QJsonObject obj = value.toObject();
		if(!value["active"].toBool())
			continue;
		Chat* c = new Chat(obj);
		insert(c);
		emit added(c->get_chatType(), c->get_id(), m_chats.count() - 1);
	}
	endInsertRows();

	// TODO: emit channel loaded?, request latest 24hrs
}

Chat* ChatsModel::get(int row) const
{
	if(row < 0) return nullptr;
	return m_chats[row];
}

void ChatsModel::remove(int row)
{
	m_chats[row]->leave();
	m_chatMap.remove(m_chats[row]->get_id());
	beginRemoveRows(QModelIndex(), row, row);
	delete m_chats[row];
	m_chats.remove(row);
	endRemoveRows();
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
			beginInsertRows(QModelIndex(), rowCount(), rowCount());
			Chat* newChat = new Chat(chatJson);
			insert(newChat);
			emit added(newChat->get_chatType(), newChat->get_id(), m_chats.count() - 1);
			endInsertRows();
		}
		// TODO: tell @cammellos that the messages are not returning the ens name
		m_contacts->upsert(m_chatMap[chatId]->get_lastMessage());
	}

	// Messages
	foreach(QJsonValue msgJson, updates["messages"].toArray())
	{
		Message* message = new Message(msgJson);
		m_chatMap[message->get_chatId()]->get_messages()->push(message);

		// Create a contact if necessary
		m_contacts->upsert(message);
	}

	// Emoji reactions*/
}