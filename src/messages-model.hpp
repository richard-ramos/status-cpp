#pragma once

#include "message.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QVector>
#include "contacts-model.hpp"

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
		// Sticker
		Clock = Qt::UserRole + 6,
		ChatId = Qt::UserRole + 7,
		SectionIdentifier = Qt::UserRole + 8,
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

	explicit MessagesModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	void push(Message* message);
	
	QML_WRITABLE_PROPERTY(ContactsModel*, contacts)


private:
	QVector<Message*> m_messages;
};