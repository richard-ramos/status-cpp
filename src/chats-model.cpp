#include <QAbstractListModel>
#include "chats-model.hpp"
#include "chat.hpp"
#include "status.hpp"
#include "chat-type.hpp"
#include <QDebug>
#include "status.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <algorithm>
#include <array>


ChatsModel::ChatsModel(QObject * parent): QAbstractListModel(parent)
{
    loadChats();
    QObject::connect(Status::instance(), &Status::logout, this, &ChatsModel::terminate);
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
    roles[Id] = "id";
    roles[Name] = "name";
    roles[Muted] = "muted";
    roles[Identicon] = "identicon";
    roles[UnreadMessages] = "unviewedMessagesCount";
    roles[HasMentions] = "hasMentions";
    roles[ContentType] = "contentType";
    roles[Type] = "chatType";
    roles[Color] = "color";
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

        // TODO: case HasMentions: return QVariant(chat->get_hasMentions());
        //TODO: case ContentType: return QVariant(chat->get_contentType());
    }

    return QVariant();
}


void ChatsModel::join(ChatType chatType, QString id)
{
    auto chatExists = std::find_if(m_chats.begin(), m_chats.end(), [chatType, id](const Chat* m) -> bool { return m->get_id() == id && m->get_chatType() == chatType; });
    if (chatExists == m_chats.end()) {
        qDebug() << "ChatsModel::join - Chat does not exist. Creating chat";
        try {
            Chat* c = new Chat(id, chatType);
            c->save();
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            m_chats << c;
            endInsertRows();
            emit joined(chatType, id, m_chats.count() - 1);
        } catch (const std::exception& e) {
            qWarning() << "ChatsModel::join - Error saving chat: " << e.what();
            emit joinError(e.what());
        }
    }
    //swithc active chat
}


void ChatsModel::loadChats()
{
    const auto response = Status::instance()->callPrivateRPC("wakuext_chats", QJsonArray{}.toVariantList()).toJsonObject();
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    foreach (const QJsonValue & value, response["result"].toArray()) {
        const QJsonObject obj = value.toObject();
        Chat* c = new Chat(obj, this);
        m_chats << c;
        emit added(c->get_chatType(), c->get_id(), m_chats.count() - 1);
    }
    endInsertRows();

    // TODO: emit channel loaded?, request latest 24hrs
    // TODO: connect loaded to added?
}