#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QRandomGenerator>
#include <QUuid>
#include <QtConcurrent>

#include <algorithm>
#include <array>

#include "message.hpp"
#include "messages-model.hpp"
#include "contacts-model.hpp"
#include "status.hpp"

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
	return roles;
}

int MessagesModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_messages.size();
}

QString sectionIdentifier(const Message* msg){
	if(msg->get_contentType() == ContentType::Group){
		return "GroupChatMessage";
	} else {
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

	switch (role)
    {
        case Id: return QVariant(msg->get_id());
		case PlainText: 
			return QVariant(msg->get_text());
		case Contact:
			return QVariant(QVariant::fromValue(m_contacts->get(msg->get_from())));
		case ContentType:
			return QVariant(msg->get_contentType());
		case Clock:
			return QVariant(msg->get_clock());
		case ChatId:
			return QVariant(msg->get_chatId());
		case Timestamp:
			return QVariant(msg->get_timestamp());
		case SectionIdentifier:
			return QVariant(sectionIdentifier(msg));
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