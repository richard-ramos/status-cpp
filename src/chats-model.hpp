#ifndef CHATSMODEL_H
#define CHATSMODEL_H

#include <QAbstractListModel>
#include "chat.hpp"
#include "chat-type.hpp"

/*
class Message : public QObject
{
Q_OBJECT
public:
    explicit Message(QObject * parent = nullptr);
}


class MessageModel : public QAbstractListModel
{
Q_OBJECT

public:
    enum MessageModel {
        Id = Qt::UserRole + 1,
        
    };

    explicit MessageModel(QObject * parent = nullptr);

    QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

}
*/

class ChatsModel : public QAbstractListModel
{
Q_OBJECT

public:
    enum ChatRoles {
        Id = Qt::UserRole + 1,
        Name = Qt::UserRole + 2,
        Type = Qt::UserRole + 3,
        Color = Qt::UserRole + 4,
        Identicon = Qt::UserRole + 5,
        Timestamp = Qt::UserRole + 6,
        Muted = Qt::UserRole + 7,
        ContentType = Qt::UserRole + 8,
        LastMessage = Qt::UserRole + 9,
        UnreadMessages = Qt::UserRole + 10,
        HasMentions = Qt::UserRole + 11,
        Description = Qt::UserRole + 12
    };

    explicit ChatsModel(QObject * parent = nullptr);
    

    QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    // Q_INVOKABLE void init();
    Q_INVOKABLE void join(ChatType chatType, QString id);
    Q_INVOKABLE void terminate();

    //Q_INVOKABLE void update();
    //Q_INVOKABLE void leave(chatType, id);
    //Q_INVOKABLE void mute(chatType, id);

signals:
    void joinError(QString message);
    void joined(ChatType chatType, QString id);
    
private:
    void loadChats();

    QVector<Chat*> m_chats;
};

#endif // CHATSMODEL_H