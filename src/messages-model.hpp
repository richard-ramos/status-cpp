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
		Text = Qt::UserRole + 2,
		Contact = Qt::UserRole + 3
		
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