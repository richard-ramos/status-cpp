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
	roles[Text] = "text";
	roles[Contact] = "contact";


	// TODO: roles[From] = fromVariant(messagesmodel->get[message->get_from()])

	return roles;
}

int MessagesModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_messages.size();
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
		case Text: 
			return QVariant(msg->get_text());
		case Contact:
			return QVariant(QVariant::fromValue(m_contacts->get(msg->get_from())));
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