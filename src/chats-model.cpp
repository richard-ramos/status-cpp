#include <QAbstractListModel>
#include "chats-model.hpp"
#include "chat.hpp"
#include "utils.hpp"
#include "status.hpp"
#include "chat-type.hpp"
#include <QDebug>
#include "status.hpp"
#include "message.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <algorithm>
#include <array>
#include <QQmlApplicationEngine>


ChatsModel::ChatsModel(QObject * parent): QAbstractListModel(parent)
{
    startMessenger();
    loadChats();
    QObject::connect(Status::instance(), &Status::logout, this, &ChatsModel::terminate);
    QObject::connect(Status::instance(), &Status::message, this, &ChatsModel::update);
    QObject::connect(this, &ChatsModel::joined, this, &ChatsModel::added);

}


void ChatsModel::terminate()
{
    qDebug() << "ChatsModel::terminate - Deleting chats";
    qDeleteAll(m_chats);
    m_chats.clear();
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

    return roles;
}


int ChatsModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return m_chats.size();
}


QVariant ChatsModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    Chat* chat = m_chats[index.row()];

    switch (role)
    {
        case Id: return QVariant(chat->get_id());
        case Name: return QVariant(chat->get_name());
        case Muted: return QVariant(chat->get_muted());
        case Identicon: return QVariant(chat->get_identicon());
        case UnreadMessages: return QVariant(chat->get_unviewedMessagesCount());
        case Type: return QVariant(chat->get_chatType());
        case Color: return QVariant(chat->get_color());
        case Timestamp: return QVariant(chat->get_timestamp());
        case Messages: return QVariant(QVariant::fromValue(chat->get_messages()));
        
        // TODO: case HasMentions: return QVariant(chat->get_hasMentions());
        //TODO: case ContentType: return QVariant(chat->get_contentType());
    }

    return QVariant();
}


void ChatsModel::join(ChatType chatType, QString id)
{
    if (!m_chatMap.contains(id)) {
        qDebug() << "ChatsModel::join - Chat does not exist. Creating chat: " << id;
        try {
            Chat* c = new Chat(id, chatType);
            QQmlApplicationEngine::setObjectOwnership(c, QQmlApplicationEngine::CppOwnership);
            c->setParent(this);
            c->save();
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            m_chats << c;
            m_chatMap[id] =  c;
            endInsertRows();
            c->loadFilter();
            emit joined(chatType, id, m_chats.count() - 1);
        } catch (const std::exception& e) {
            qWarning() << "ChatsModel::join - Error saving chat: " << e.what();
            emit joinError(e.what());
        }
        qDebug() << "ChatsModel::join - Chat saved";
    }
}


void ChatsModel::startMessenger()
{
    const auto response = Status::instance()->callPrivateRPC("wakuext_startMessenger", QJsonArray{}.toVariantList()).toJsonObject();
    // TODO: do something with filters and ranges

}


void ChatsModel::loadChats()
{
    const auto response = Status::instance()->callPrivateRPC("wakuext_chats", QJsonArray{}.toVariantList()).toJsonObject();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    foreach (const QJsonValue & value, response["result"].toArray()) {
        const QJsonObject obj = value.toObject();
        Chat* c = new Chat(obj, this);
        QQmlApplicationEngine::setObjectOwnership(c, QQmlApplicationEngine::CppOwnership);
        c->setParent(this);
        m_chatMap[c->get_id()] = c;
        m_chats << c;
        emit added(c->get_chatType(), c->get_id(), m_chats.count() - 1);
    }
    endInsertRows();

    // TODO: emit channel loaded?, request latest 24hrs
}

Chat* ChatsModel::get(int row) const
{
    return m_chats[row];
}


void ChatsModel::update(QJsonValue updates)
{
    // Process chats
    foreach(QJsonValue chatJson, updates["chats"].toArray()){
        QString chatId = chatJson["id"].toString();
        if(m_chatMap.contains(chatId)){
            int chatIndex = -1;
            for(int i = 0; i < m_chats.count(); i++){
                if(m_chats[i]->get_id() == chatId){
                    chatIndex = i;
                    break;
                }
            }
            m_chatMap[chatId]->update(chatJson);
            if(chatIndex > -1){
                QModelIndex idx = createIndex(chatIndex,0);
                dataChanged(idx, idx);
            }
        } else {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            Chat* newChat = new Chat(chatJson, this);
            newChat->setParent(this);
            QQmlApplicationEngine::setObjectOwnership(newChat, QQmlApplicationEngine::CppOwnership);
            m_chats << newChat;
            m_chatMap[newChat->get_id()] = newChat;
            emit added(newChat->get_chatType(), newChat->get_id(), m_chats.count() - 1);
            endInsertRows();
        }
    }

    // Messages
    foreach(QJsonValue msgJson, updates["messages"].toArray()){
        Message* message = new Message(msgJson, this);
        m_chatMap[message->get_chatId()]->get_messages()->push(message);
    }

    // Emoji reactions*/
}
