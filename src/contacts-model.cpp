#include "contacts-model.hpp"
#include "contact.hpp"
#include "message.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

using namespace Messages;

ContactsModel::ContactsModel(QObject* parent)
	: QAbstractListModel(parent)
{
	loadContacts();
	QObject::connect(Status::instance(), &Status::message, this, &ContactsModel::update);
	QObject::connect(this, &ContactsModel::contactLoaded, this, &ContactsModel::push); // TODO: replace added
}

QHash<int, QByteArray> ContactsModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "contactId";
	roles[Name] = "name";
	roles[Alias] = "alias";
	roles[LocalNickname] = "localNickname";
	roles[Identicon] = "identicon";
	roles[IsAdded] = "isAdded";
	roles[IsBlocked] = "isBlocked";
	roles[Image] = "image";
	return roles;
}

int ContactsModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_contacts.size();
}

QVariant ContactsModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Contact* contact = m_contacts[index.row()];

	switch(role)
	{
	case Id: return QVariant(contact->get_id());
	case Name: return QVariant(contact->get_name());
	case Alias: return QVariant(contact->get_alias());
	case LocalNickname: return QVariant(contact->get_localNickname());
	case Identicon: return QVariant(contact->get_identicon());
	case IsAdded: return QVariant(contact->isAdded());
	case IsBlocked: return QVariant(contact->isBlocked());
	case Image: return QVariant(contact->image());
	}

	return QVariant();
}

void ContactsModel::contactUpdated(QString contactId)
{
	if(!m_contactsMap.contains(contactId))
		return;
	int index = m_contacts.indexOf(m_contactsMap[contactId]);
	QModelIndex idx = createIndex(index, 0);
	dataChanged(idx, idx);
}

void ContactsModel::loadContacts()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("wakuext_contacts", QJsonArray{}.toVariantList()).toJsonObject();

		if(response["result"].isNull())
			return;

		foreach(const QJsonValue& value, response["result"].toArray())
		{
			const QJsonObject obj = value.toObject();
			Contact* c = new Contact(obj);
			c->moveToThread(QApplication::instance()->thread());
			emit contactLoaded(c);
		}
	});
}

void ContactsModel::push(Contact* contact)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	insert(contact);
	endInsertRows();
}

void ContactsModel::insert(Contact* contact)
{
	QQmlApplicationEngine::setObjectOwnership(contact, QQmlApplicationEngine::CppOwnership);
	contact->setParent(this);
	m_contacts << contact;
	m_contactsMap[contact->get_id()] = contact;
	QObject::connect(contact, &Contact::contactToggled, this, &ContactsModel::contactUpdated);
	QObject::connect(contact, &Contact::blockedToggled, this, &ContactsModel::contactUpdated);
	QObject::connect(contact, &Contact::imageChanged, this, &ContactsModel::contactUpdated);
}

Contact* ContactsModel::get(int row) const
{
	return m_contacts[row];
}

Contact* ContactsModel::get(QString id) const
{
	return m_contactsMap[id];
}

Contact* ContactsModel::get_or_create(QString id)
{
	if(m_contactsMap.contains(id))
		return m_contactsMap[id];

	Contact* contact = new Contact(id, this);
	insert(contact);
	return contact;
}

Contact* ContactsModel::upsert(Message* msg)
{
	if(m_contactsMap.contains(msg->get_from()))
	{
		return m_contactsMap[msg->get_from()];
	}
	else
	{
		beginInsertRows(QModelIndex(), rowCount(), rowCount());
		Contact* newContact = new Contact(msg);
		insert(newContact);
		emit added(newContact->get_id());
		endInsertRows();
		return newContact;
	}
}

/*
void ContactsModel::add(QString contactId, QString ensName)
{

}*/

void ContactsModel::update(QJsonValue updates)
{
	// Process contacts
	foreach(QJsonValue contactJson, updates["contacts"].toArray())
	{
		QString contactId = contactJson["id"].toString();
		if(m_contactsMap.contains(contactId))
		{
			int contactIndex = m_contacts.indexOf(m_contactsMap[contactId]);
			m_contactsMap[contactId]->update(contactJson);
			if(contactIndex > -1)
			{
				QModelIndex idx = createIndex(contactIndex, 0);
				dataChanged(idx, idx);
			}
		}
		else
		{
			beginInsertRows(QModelIndex(), rowCount(), rowCount());
			Contact* newContact = new Contact(contactJson);
			insert(newContact);
			emit added(newContact->get_id());
			endInsertRows();
		}
	}
}
