#include "mailserver-model.hpp"
#include "mailserver-cycle.hpp"
#include "settings.hpp"
#include "status.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QSysInfo>
#include <QTimer>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

MailserverModel::MailserverModel(QObject* parent)
	: QAbstractListModel(parent)
{
	mailserverCycle = new MailserverCycle(this);
	timer = new QTimer(this);

	QObject::connect(Status::instance(), &Status::logout, timer, &QTimer::stop);
	QObject::connect(Status::instance(),
					 &Status::discoverySummary,
					 mailserverCycle,
					 &MailserverCycle::peerSummaryChange); // TODO: determine if i need to preload peers?
	QObject::connect(this, &MailserverModel::mailserverLoaded, this, &MailserverModel::push);
	QObject::connect(mailserverCycle, &MailserverCycle::requestSent, this, &MailserverModel::mailserverRequestSent);
	QObject::connect(mailserverCycle, &MailserverCycle::activeMailserverChanged, this, &MailserverModel::activeMailserverChanged);

	loadMailservers();
	loadCustomMailservers();
	startMailserverCycle();
}

void MailserverModel::startMailserverCycle()
{
	// Fire immediately
	mailserverCycle->work();
	// Execute every 10 seconds
	QObject::connect(timer, &QTimer::timeout, mailserverCycle, &MailserverCycle::work);
	timer->start(10000);
}

MailserverCycle* MailserverModel::getCycle()
{
	return mailserverCycle;
}

QHash<int, QByteArray> MailserverModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "mailserverId";
	roles[Name] = "name";
	roles[Endpoint] = "endpoint";
	return roles;
}

int MailserverModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_mailservers.size();
}

QVariant MailserverModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Mailserver mailserver = m_mailservers[index.row()];

	switch(role)
	{
	case Id: return QVariant(mailserver.id);
	case Name: return QVariant(mailserver.name);
	case Endpoint: return QVariant(mailserver.endpoint);
	}

	return QVariant();
}

void MailserverModel::loadMailservers()
{
	QtConcurrent::run([=] {
		// Getting default mailservers
		QFile fleets(":/resources/fleets.json");
		fleets.open(QIODevice::ReadOnly);
		QJsonObject fleetJson = QJsonDocument::fromJson(fleets.readAll()).object()["fleets"].toObject()[Settings::instance()->fleet()].toObject();
		auto mail = fleetJson["mail"].toObject();
		foreach(const QString& key, mail.keys())
		{
			Mailserver m{.id = "", .name = key, .endpoint = mail[key].toString()};
			emit mailserverLoaded(m);
		}
	});
}

void MailserverModel::enableAutomaticSelection(bool enable)
{
	if(enable)
	{
		qDebug() << "Restarting mailserver cicle ";
		timer->stop();
		Settings::instance()->setPinnedMailserver("");
		startMailserverCycle();
	}
}

void MailserverModel::pinMailserver(QString endpoint)
{
	qDebug() << "Pinning " << endpoint;
	timer->stop();
	Settings::instance()->setPinnedMailserver(endpoint);
	startMailserverCycle();
}

void MailserverModel::add(QString name, QString endpoint)
{
	QString fleet = Settings::instance()->fleet();
	QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
	const auto response =
		Status::instance()
			->callPrivateRPC("mailservers_addMailserver",
							 QJsonArray{QJsonObject{{"id", id}, {"name", name}, {"address", endpoint}, {"fleet", fleet}}}.toVariantList())
			.toJsonObject();
	// TODO: error handling
	Mailserver m{.id = id, .name = name, .endpoint = endpoint};
	emit mailserverLoaded(m);
}

void MailserverModel::loadCustomMailservers()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("mailservers_getMailservers", QJsonArray{}.toVariantList()).toJsonObject();

		if(response["result"].isNull()) return;

		QString fleet = Settings::instance()->fleet();

		foreach(const QJsonValue& value, response["result"].toArray())
		{
			const QJsonObject obj = value.toObject();
			if(obj["fleet"].toString() != fleet) continue;

			Mailserver t{.id = obj["id"].toString(), .name = obj["name"].toString(), .endpoint = obj["endpoint"].toString()};
			emit mailserverLoaded(t);
		}
	});
}

void MailserverModel::push(Mailserver mailserver)
{
	insert(mailserver);
}

void MailserverModel::insert(Mailserver mailserver)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_mailservers << mailserver;
	endInsertRows();
}

Mailserver MailserverModel::getActiveMailserver()
{
	QString activeMailserverEndpoint = mailserverCycle->getActiveMailserver();
	foreach(Mailserver m, m_mailservers)
	{
		if(m.endpoint == activeMailserverEndpoint)
		{
			return m;
		}
	}
	return Mailserver{};
}