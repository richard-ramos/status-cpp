#pragma once

#include "chat-type.hpp"
#include "chat.hpp"
#include "contacts-model.hpp"
#include "message.hpp"
#include "mailserver-model.hpp"
#include "mailserver-cycle.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QHash>
#include <QQmlHelpers>
#include <QVariantList>
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
		Messages = Qt::UserRole + 13,
		Contact = Qt::UserRole + 14,
		ChatMembers = Qt::UserRole + 15
	};

	explicit ChatsModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void init();
	Q_INVOKABLE void join(ChatType chatType, QString id, QString ensName = "");
	Q_INVOKABLE void createGroup(QString groupName, QStringList members);

	Q_INVOKABLE Chat* get(int row) const;
	Q_INVOKABLE void remove(int row);
	Q_INVOKABLE void markAllMessagesAsRead(int row);
	Q_INVOKABLE void deleteChatHistory(int row);

	Q_PROPERTY(QVariant timelineMessages READ timelineMessages CONSTANT)
	Q_INVOKABLE QVariant timelineMessages();
	Q_INVOKABLE void toggleTimelineChat(QString contactId, bool contactWasAdded);
	Q_INVOKABLE void pushStatusUpdate(Message* msg);

	QML_WRITABLE_PROPERTY(ContactsModel*, contacts)
	QML_WRITABLE_PROPERTY(MailserverModel*, mailservers)

	Q_INVOKABLE void onContactsChanged();
	Q_INVOKABLE void onMailserversChanged();

signals:
	void joinError(QString message);
	void joined(ChatType chatType, QString id, int index);
	void added(ChatType chatType, QString id, int index);
	void left(int index);
	void lastRequest(ChatType chatType, QString id, qint64 lastRequest);

private:
	void startMessenger();
	void loadChats();
	void update(QJsonValue updates);
	void insert(Chat* chat);
	void addTimelineChat();
	void removeFilter(Chat* chat);

	void removeTimelineMessages(QString contactId);
	void remove1on1Filters(QString chatId, QJsonArray filters);
	void removeFilterRPC(QString chatId, QString filterId);

	QVector<Chat*> m_chats;
	QVector<Chat*> m_timelineChats;
	QHash<QString, Chat*> m_chatMap;
};
