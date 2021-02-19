#pragma once

#include "message.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QVector>
#include <QHash>
#include "contacts-model.hpp"

using namespace Messages;


class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum MessageRoles
	{
		Id = Qt::UserRole + 1,
		PlainText = Qt::UserRole + 2,
		Contact = Qt::UserRole + 3,
		Timestamp = Qt::UserRole + 4,
		ContentType = Qt::UserRole + 5,
		Clock = Qt::UserRole + 6,
		ChatId = Qt::UserRole + 7,
		SectionIdentifier = Qt::UserRole + 8,
		ParsedText = Qt::UserRole + 9,
		Sticker = Qt::UserRole + 10,
		ResponseTo = Qt::UserRole + 11
		// OutgoingStatus
		// ResponseTo
		// Index?
		// Image
		// Audio
		// AUdioDurationMs
		// EmojiReactions
		// CommandParameters
		// LinkUrls
		// CommunityId
	};

	explicit MessagesModel(QString chatId, QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	void push(Message* message);
	Q_INVOKABLE Message* get(QString messageId) const;
	Q_INVOKABLE Message* get(int row) const;

	
	QML_WRITABLE_PROPERTY(ContactsModel*, contacts)

	QString renderBlock(Message* message) const;

public:
	void loadMessages(bool initialLoad = true);
	

signals:
	void messageLoaded(Message* message);

private:
	QVector<Message*> m_messages;
	QHash<QString, Message*> m_messageMap;
	QString m_chatId;

	QString m_cursor;

	void addFakeMessages();

};