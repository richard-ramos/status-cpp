#pragma once

#include "message.hpp"
#include <QAbstractListModel>
#include <QDebug>
#include <QVector>

class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum MessageRoles
	{
		Id = Qt::UserRole + 1,
	};

	explicit MessagesModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	void push(Message* message);
	void terminate();

private:
	QVector<Message*> m_messages;
};