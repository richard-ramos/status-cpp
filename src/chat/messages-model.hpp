#pragma once

#include "chat-type.hpp"
#include "contacts-model.hpp"
#include "message.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QHash>
#include <QMutex>
#include <QQmlHelpers>
#include <QVector>

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
		ResponseTo = Qt::UserRole + 11,
		LinkUrls = Qt::UserRole + 12,
		OutgoingStatus = Qt::UserRole + 13,
		Image = Qt::UserRole + 14,
		EmojiReactions = Qt::UserRole + 15,
		HasMention = Qt::UserRole + 16
		// Audio
		// AUdioDurationMs
		// CommandParameters
		// CommunityId
	};

	explicit MessagesModel(QString chatId, ChatType chatType, QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	void push(Message* message);
	void push(QString messageId, QJsonObject reaction);

	Q_INVOKABLE Message* get(QString messageId) const;
	Q_INVOKABLE Message* get(int row) const;
	Q_INVOKABLE void toggleReaction(QString messageId, int emojiId);
	Q_INVOKABLE void updateOutgoingStatus(QVector<QString> messageIds, bool sent);
	Q_INVOKABLE void resend(QString messageId);

	QML_WRITABLE_PROPERTY(ContactsModel*, contacts)

	QString renderBlock(Message* message) const;

public:
	void loadMessages(bool initialLoad = true);
	void loadReactions(bool initialLoad = true);
	void clear();

signals:
	void statusUpdateLoaded(Message* message);
	void messageLoaded(Message* message);
	void reactionLoaded(QString messageId, QJsonObject reaction);

private:
	QVector<Message*> m_messages;
	QHash<QString, Message*> m_messageMap;
	QHash<QString, QJsonArray> m_emojiReactions;
	QString m_chatId;
	ChatType m_chatType;

	QString m_cursor;
	QString m_reactionsCursor;
	QMutex m_mutex;

	void addFakeMessages();
};