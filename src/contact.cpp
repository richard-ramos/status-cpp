#include "contact.hpp"
#include "chat-type.hpp"
#include "chat.hpp"
#include "message.hpp"
#include "messages-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QDateTime>
#include <QDebug>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QRandomGenerator>
#include <QString>
#include <QVariant>
#include <QtConcurrent>
#include <stdexcept>

Contact::Contact(QString id, QObject* parent)
	: QObject(parent)
	, m_id(id)
{ }

Contact::Contact(Message* msg, QObject* parent)
	: QObject(parent)
	, m_id(msg->get_from())
	, m_name(msg->get_ensName())
	, m_alias(msg->get_alias())
	, m_identicon(msg->get_identicon())
{
	qDebug() << "Contact::Contact - Contact does not exist. Creating from message - " << msg->get_from();
}

Contact::~Contact()
{
	m_systemTags.clear();
}

bool Contact::operator==(const Contact& c)
{
	return m_id == c.get_id();
}

Contact::Contact(const QJsonValue data, QObject* parent)
	: QObject(parent)
	, m_id(data["id"].toString())
	, m_address(data["address"].toString())
	, m_name(data["name"].toString())
	, m_ensVerified(data["ensVerified"].toBool())
	, m_ensVerifiedAt(data["ensVerifiedAt"].toString())
	, m_lastENSClockValue(data["lastENSClockValue"].toString())
	, m_ensVerificationRetries(data["ensVerificationRetries"].toString())
	, m_alias(data["alias"].toString())
	, m_identicon(data["identicon"].toString())
	, m_lastUpdated(data["lastUpdated"].toString())
	, m_tributeToTalk(data["tributeToTalk"].toString())
	, m_localNickname(data["localNickname"].toString())
{
	foreach(const QJsonValue& value, data["systemTags"].toArray())
	{
		m_systemTags << value.toString();
	}
}

void Contact::changeNickname(QString newNickname)
{
	update_localNickname(newNickname);
	save();
}

bool Contact::isAdded()
{
	QString tag(":contact/added");
	return m_systemTags.indexOf(tag) > -1;
}

bool Contact::isBlocked()
{
	// TODO: DRY
	QString tag(":contact/blocked");
	return m_systemTags.indexOf(tag) > -1;
}

void Contact::toggleAdd()
{
	QString tag(":contact/added");
	int index = m_systemTags.indexOf(tag);
	if(index == -1){
		m_systemTags << tag;
	} else {
		m_systemTags.remove(index);
	}
	emit contactToggled(m_id);
	save();

	// TODO: react to contactToggled to add/remove timeline chat
}

void Contact::toggleBlock()
{
	// TODO: DRY
	QString tag(":contact/blocked");
	int index = m_systemTags.indexOf(tag);
	if(index == -1){
		m_systemTags << tag;
	} else {
		m_systemTags.remove(index);
	}
	emit blockedToggled(m_id);
	save();

	// TODO: react to blockedToggled to add/remove timeline chat
}

void Contact::save()
{
	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);

		QJsonArray systemTagsArr;
		for (auto & tag : m_systemTags)
			systemTagsArr.append(tag);


		QJsonObject contact{{"id", m_id},
							{"address", m_address},
							{"name", m_name},
							{"ensVerified", m_ensVerified},
							{"ensVerifiedAt", m_ensVerifiedAt.toLongLong()},
							{"lastENSClockValue", m_lastENSClockValue.toLongLong()},
							{"ensVerificationRetries", m_ensVerificationRetries.toLongLong()},
							{"alias", m_alias},
							{"identicon", m_identicon},
							{"lastUpdated", m_lastUpdated.toLongLong()},
							{"tributeToTalk", m_tributeToTalk},
							{"systemTags", systemTagsArr},
							// TODO: DeviceInfo ???
							// TODO: images ???
							{"localNickname", m_localNickname}};

		const auto response =
			Status::instance()
				->callPrivateRPC("wakuext_saveContact", QJsonArray{contact}.toVariantList())
				.toJsonObject();
		qDebug() << response;
		if(!response["error"].isUndefined())
		{
			throw std::domain_error(response["error"]["message"].toString().toUtf8());
		}
	});
}

void Contact::update(const QJsonValue data)
{
	// Data is old
	if(m_lastUpdated.toLongLong() > data["lastUpdated"].toString().toLongLong())
		return;

	update_address(data["address"].toString());
	update_name(data["name"].toString());
	update_ensVerified(data["ensVerified"].toBool());
	update_ensVerifiedAt(data["ensVerifiedAt"].toString());
	update_lastENSClockValue(data["lastENSClockValue"].toString());
	update_ensVerificationRetries(data["ensVerificationRetries"].toString());
	update_lastUpdated(data["lastUpdated"].toString());
	update_tributeToTalk(data["tributeToTalk"].toString());
	update_localNickname(data["localNickname"].toString());

	// TODO: test this when syncing
	m_systemTags.clear();
	foreach(const QJsonValue& value, data["systemTags"].toArray())
	{
		m_systemTags << value.toString();
	}
}
