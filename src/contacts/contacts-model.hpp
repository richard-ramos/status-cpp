#pragma once

#include "message.hpp"
#include "contact.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QVector>

class Chat;

using namespace Messages;

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
		IsBlocked = Qt::UserRole + 5,
		Image = Qt::UserRole + 6,
		LocalNickname = Qt::UserRole + 7,
		Alias = Qt::UserRole + 8,
		Address = Qt::UserRole + 9,
		EnsVerified = Qt::UserRole + 10
	};

	explicit ContactsModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	int count() const;
	Q_INVOKABLE Contact* get(int row) const;
	Q_INVOKABLE Contact* get(QString id) const;
	Q_INVOKABLE Contact* get_or_create(QString id);
	Q_INVOKABLE void contactUpdated(QString id);
	Q_INVOKABLE void push(Contact* contact);

	Contact* upsert(Message* msg);
	Contact* upsert(Chat* chat);

signals:
	void updated(QString contactId);
	void added(QString contactId);
	void contactLoaded(Contact* contact);
	void contactToggled(QString contactId, bool added);
private:
	void loadContacts();
	void update(QJsonValue updates);
	void insert(Contact* contact);

	QVector<Contact*> m_contacts;
	QHash<QString, Contact*> m_contactsMap;
};
