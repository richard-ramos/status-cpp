#include "messages-model.hpp"
#include "contacts-model.hpp"
#include "content-type.hpp"
#include "message-format.hpp"
#include "message.hpp"
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
#include <algorithm>
#include <array>

using namespace Messages;

MessagesModel::MessagesModel(QString chatId, QObject* parent)
	: m_chatId(chatId)
	, QAbstractListModel(parent)
{
	qDebug() << "MessagesModel::constructor for chatId: " << m_chatId;
	QObject::connect(this, &MessagesModel::messageLoaded, this, &MessagesModel::push); // TODO: replace added

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
			delete m_messageMap[msg->get_id()];
			m_messages.remove(row);
			endRemoveRows();
		}
	}

	if(m_messageMap.contains(msg->get_id()))
		return;

	m_contacts->upsert(msg);

	QQmlApplicationEngine::setObjectOwnership(msg, QQmlApplicationEngine::CppOwnership);
	msg->setParent(this);
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_messageMap[msg->get_id()] = msg;
	m_messages << msg;
	endInsertRows();
}

void MessagesModel::loadMessages(bool initialLoad)
{
	if(!initialLoad && m_cursor == "")
		return;

	QtConcurrent::run([=] {
		const auto response =
			Status::instance()->callPrivateRPC("wakuext_chatMessages", QJsonArray{m_chatId, m_cursor, 20}.toVariantList()).toJsonObject();
		m_cursor = response["result"]["cursor"].toString();

		// TODO: handle cursor
		foreach(QJsonValue msgJson, response["result"]["messages"].toArray())
		{
			Message* message = new Message(msgJson);
			message->moveToThread(QApplication::instance()->thread());
			emit messageLoaded(message);
		}
	});
}

void MessagesModel::addFakeMessages()
{
	Message* chatIdentifier = new Message("chatIdentifier", ContentType::ChatIdentifier, this);
	QQmlApplicationEngine::setObjectOwnership(chatIdentifier, QQmlApplicationEngine::CppOwnership);
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_messages << chatIdentifier;
	endInsertRows();
}

void MessagesModel::clear()
{
	beginResetModel();
	m_messages.clear();
	addFakeMessages();
	endResetModel();
}