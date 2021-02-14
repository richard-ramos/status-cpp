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
#include <QtConcurrent>
#include <algorithm>
#include <array>

#include "contacts-model.hpp"
#include "content-type.hpp"
#include "message.hpp"
#include "messages-model.hpp"
#include "message-format.hpp"
#include "status.hpp"
#include "utils.hpp"

using namespace Messages;

MessagesModel::MessagesModel(QObject* parent)
	: QAbstractListModel(parent)
{
	qDebug() << "MessagesModel::constructor";
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
	case PlainText: return QVariant(msg->get_text());
	case Contact: return QVariant(QVariant::fromValue(m_contacts->get(msg->get_from())));
	case ContentType: return QVariant(msg->get_contentType());
	case Clock: return QVariant(msg->get_clock());
	case ChatId: return QVariant(msg->get_chatId());
	case Timestamp: return QVariant(msg->get_timestamp());
	case SectionIdentifier: return QVariant(sectionIdentifier(msg));
	case ParsedText: return QVariant(Messages::Format::renderBlock(msg, m_contacts));
	case Sticker: return QVariant(Messages::Format::decodeSticker(msg));
	}

	return QVariant();
}

void MessagesModel::push(Message* msg)
{
	QQmlApplicationEngine::setObjectOwnership(msg, QQmlApplicationEngine::CppOwnership);
	msg->setParent(this);
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_messages << msg;
	endInsertRows();
}
