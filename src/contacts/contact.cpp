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
	, m_alias(Utils::generateAlias(id))
	, m_identicon(Utils::generateIdenticon(id))
{
	qDebug() << "Contact does not exist:" << m_alias;
}

Contact::Contact(QString id, QString ensName, QObject* parent)
	: QObject(parent)
	, m_id(id)
	, m_alias(Utils::generateAlias(id))
	, m_identicon(Utils::generateIdenticon(id))
	, m_name(ensName)
{
	if(!m_name.isEmpty()){
		m_ensVerified = true;
	}
	qDebug() << "Contact does not exist:" << m_alias;
}

Contact::~Contact()
{
	m_systemTags.clear();
	m_images.clear();
}

bool Contact::operator==(const Contact& c)
{
	return m_id == c.get_id();
}

QVector<QString> Contact::getSystemTags(){
	return m_systemTags;
}

QVector<ContactImage> Contact::getImages(){
	return m_images;
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

	// TODO: determine if it's possible to have more than one image
	if(data["images"].toObject()["thumbnail"].isNull())
		return;
	ContactImage image;
	image.type = data["images"].toObject()["thumbnail"].toObject()["type"].toString();
	image.uri = data["images"].toObject()["thumbnail"].toObject()["uri"].toString();
	m_images << image;
}

QString Contact::image()
{
	if(m_images.size() > 0)
	{
		return m_images[0].uri;
	}
	else
	{
		return m_identicon;
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
	bool added = false;
	if(index == -1)
	{
		m_systemTags << tag;
		added = true;
	}
	else
	{
		m_systemTags.remove(index);
	}
	emit contactToggled(m_id, added);
	save();

	// TODO: react to contactToggled to add/remove timeline chat
}

void Contact::toggleBlock()
{
	// TODO: DRY
	QString tag(":contact/blocked");
	int index = m_systemTags.indexOf(tag);
	if(index == -1)
	{
		m_systemTags << tag;
	}
	else
	{
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
		for(auto& tag : m_systemTags)
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

		const auto response = Status::instance()->callPrivateRPC("wakuext_saveContact", QJsonArray{contact}.toVariantList()).toJsonObject();
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

	if(data["images"].toObject()["thumbnail"].isNull())
		return;
	ContactImage image;
	image.type = data["images"].toObject()["thumbnail"].toObject()["type"].toString();
	image.uri = data["images"].toObject()["thumbnail"].toObject()["uri"].toString();
	m_images << image;
	imageChanged(m_id);
}

void Contact::update(Contact* newContact)
{
	// Data is old
	if(m_lastUpdated.toLongLong() > newContact->get_lastUpdated().toLongLong())
		return;

	update_address(newContact->get_address());
	update_name(newContact->get_name());
	update_ensVerified(newContact->get_ensVerified());
	update_ensVerifiedAt(newContact->get_ensVerifiedAt());
	update_lastENSClockValue(newContact->get_lastENSClockValue());
	update_ensVerificationRetries(newContact->get_ensVerificationRetries());
	update_lastUpdated(newContact->get_lastUpdated());
	update_tributeToTalk(newContact->get_tributeToTalk());
	update_localNickname(newContact->get_localNickname());

	m_images = newContact->getImages();
	m_systemTags = newContact->getSystemTags();


	// TODO: find a way to trigger these signals if values are different
	emit contactToggled(m_id, isAdded());
	emit blockedToggled(m_id);
	emit imageChanged(m_id);
}
