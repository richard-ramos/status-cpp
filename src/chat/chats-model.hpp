#pragma once

#include "chat-type.hpp"
#include "chat.hpp"
#include "contacts-model.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QHash>
#include <QQmlHelpers>
#include <QVector>

class ChatsModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum ChatRoles
	{
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
		Description = Qt::UserRole + 12,
		Messages = Qt::UserRole + 13
	};

	explicit ChatsModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void init();
	Q_INVOKABLE void join(ChatType chatType, QString id);
	Q_INVOKABLE Chat* get(int row) const;
	Q_INVOKABLE void remove(int row);
	Q_INVOKABLE void markAllMessagesAsRead(int row);
	Q_INVOKABLE void deleteChatHistory(int row);

	QML_WRITABLE_PROPERTY(ContactsModel*, contacts)

	Q_INVOKABLE void setupMessageModel();
	//Q_INVOKABLE void update();
	//Q_INVOKABLE void leave(chatType, id);
	//Q_INVOKABLE void mute(chatType, id);

signals:
	void joinError(QString message);
	void joined(ChatType chatType, QString id, int index);
	void added(ChatType chatType, QString id, int index);
	void left(int index);

private:
	void startMessenger();
	void loadChats();
	void update(QJsonValue updates);
	void insert(Chat* chat);

	QVector<Chat*> m_chats;
	QHash<QString, Chat*> m_chatMap;
};
