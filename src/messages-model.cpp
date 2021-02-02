#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QCryptographicHash>
#include <QUuid>
#include <QRandomGenerator>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QQmlApplicationEngine>

#include <algorithm>
#include <array>

#include "messages-model.hpp"
#include "message.hpp"
#include "status.hpp"


MessagesModel::MessagesModel(QObject * parent): QAbstractListModel(parent)
{
    QObject::connect(Status::instance(), &Status::logout, this, &MessagesModel::terminate);
    qDebug() << "MessagesModel::constructor";
}


void MessagesModel::terminate()
{
    qDebug() << "MessagesModel::terminate - Deleting messages";
    qDeleteAll(m_messages);
    m_messages.clear();
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id] = "messageId";
    return roles;
}

int MessagesModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return m_messages.size();
}


QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

   /* switch (role)
    {
        case Id: return QVariant(m_messages[index.row()]->get_id());
    }*/

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