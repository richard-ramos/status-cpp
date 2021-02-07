#pragma once

#include "contact.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QVector>

class ContactsModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum ContactRoles
	{
		Id = Qt::UserRole + 1,
		Name = Qt::UserRole + 2,
		Identicon = Qt::UserRole + 3,
		IsAdded = Qt::UserRole + 4,
		IsBlocked = Qt::UserRole + 5
	};

	explicit ContactsModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE Contact* get(int row) const;
	Q_INVOKABLE Contact* get(QString id) const;
	Q_INVOKABLE void contactUpdated(QString id);

	Contact* upsert(Message* msg);

signals:
	void updated(QString contactId);
	void added(QString contactId);

private:
	void loadContacts();
	void update(QJsonValue updates);
	void insert(Contact* contact);

	QVector<Contact*> m_contacts;
	QHash<QString, Contact*> m_contactsMap;
};
