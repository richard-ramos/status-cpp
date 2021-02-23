#include "mailserver-cycle.hpp"
#include "libstatus.h"
#include "settings.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QDebug>
#include <QHash>
#include <QMutex>
#include <QRandomGenerator>
#include <QThread>
#include <QtConcurrent>
#include <algorithm>
#include <cmath>

#include <QMutexLocker>

MailserverCycle::MailserverCycle(QObject* parent)
	: QThread(parent)
{ }

MailserverCycle::~MailserverCycle()
{
	wait();
	qDebug() << "Stopping mailserver cycle thread";
}

void MailserverCycle::work()
{
	if(!isRunning())
	{
		start(LowPriority);
	}
}

void MailserverCycle::trustPeer(QString enode)
{
	const auto response = Status::instance()->callPrivateRPC("waku_markTrustedPeer", QJsonArray{enode}.toVariantList());
	nodes[enode] = MailserverStatus::Trusted;
}

void MailserverCycle::updateMailserver(QString enode)
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_updateMailservers", QJsonArray{enode}.toVariantList());
}

void MailserverCycle::connect(QString enode)
{
	qDebug() << "Connecting to mailserver: " << enode;

	bool mailserverTrusted = false;

	if(!getMailservers().contains(enode))
	{
		qWarning() << "Mailserver not known";
		return;
	}

	update_activeMailserver(enode);

	// Adding a peer and marking it as trusted can't be executed sync, because
	// There's a delay between requesting a peer being added, and a signal being
	// received after the peer was added. So we first set the peer status as
	// Connecting and once a peerConnected signal is received, we mark it as
	// Connected and then as Trusted

	if(nodes.contains(enode) && nodes[enode] == MailserverStatus::Connected)
	{
		trustPeer(enode);
		updateMailserver(enode);
		mailserverTrusted = true;
	}
	else
	{
		// Attempt to connect to mailserver by adding it as a peer
		nodes[enode] = MailserverStatus::Connecting;
		const auto response = Status::instance()->callPrivateRPC("admin_addPeer", QJsonArray{enode}.toVariantList());

		QtConcurrent::run([=] {
			QThread::sleep(3); // Timeout connection attempt at 3 seconds
			timeoutConnection(enode);
		});
	}

	if(mailserverTrusted)
	{
		qDebug() << "Mailserver Available!";
		emit mailserverAvailable();
	}
}

void MailserverCycle::timeoutConnection(QString enode)
{
	QMutexLocker locker(&m_mutex);
	if(nodes[enode] != MailserverStatus::Connecting)
		return;

	qDebug() << "Mailserver connection attempt failed due to timeout";
	nodes[enode] == MailserverStatus::Disconnected;
	if(get_activeMailserver() == enode)
		update_activeMailserver("");
}

int poolSize(int fleetSize)
{
	return static_cast<int>(std::ceil(fleetSize / 4));
}

QVector<QString> MailserverCycle::getMailservers()
{
	QVector<QString> result;
	// TODO: get custom mailservers
	foreach(const QJsonValue& value, Settings::instance()->getNodeConfig()["ClusterConfig"].toObject()["TrustedMailServers"].toArray())
	{
		result << value.toString();
	}
	return result;
}

void MailserverCycle::findNewMailserver()
{
	qDebug() << "Finding a new mailserver...";

	const auto pingResponse =
		Status::instance()
			->callPrivateRPC("mailservers_ping",
							 QJsonArray{QJsonObject{{"addresses", Utils::toJsonArray(getMailservers())}, {"timeoutMs", 500}}}.toVariantList())
			.toJsonObject();

	QVector<std::tuple<QString, int>> availableMailservers;
	foreach(const QJsonValue& value, pingResponse["result"].toArray())
	{
		if(!value["error"].isNull())
			continue;
		availableMailservers << std::make_tuple(value["address"].toString(), value["rttMs"].toInt());
	}

	if(availableMailservers.count() == 0)
	{
		qWarning() << "No mailservers available";
		return;
	}

	std::sort(availableMailservers.begin(), availableMailservers.end(), [](std::tuple<QString, int> a, std::tuple<QString, int> b) {
		return std::get<1>(a) > std::get<1>(b);
	});

	// Picks a random mailserver amongs the ones with the lowest latency
	// The pool size is 1/4 of the mailservers were pinged successfully
	int poolN = poolSize(availableMailservers.count());
	QString mailServer = std::get<0>(availableMailservers[QRandomGenerator::global()->bounded(poolN)]);

	connect(mailServer);
}

void MailserverCycle::disconnectActiveMailserver()
{
	qDebug() << "Disconnecting active mailserver: " << get_activeMailserver();
	nodes[get_activeMailserver()] = MailserverStatus::Disconnected;
	const auto pingResponse =
		Status::instance()->callPrivateRPC("admin_removePeer", QJsonArray{get_activeMailserver()}.toVariantList()).toJsonObject();
	update_activeMailserver("");
}

void MailserverCycle::run()
{
	QMutexLocker locker(&m_mutex);

	// TODO: if there's a pinned mailserver, return

	if(nodes.contains(get_activeMailserver()) && nodes[get_activeMailserver()] == MailserverStatus::Trusted)
	{
		qDebug() << "Mailserver is already connected and trusted. Skipping iteration";
		return;
	}

	qDebug() << "Automatically switching mailserver";

	if(get_activeMailserver() != "")
		disconnectActiveMailserver();

	findNewMailserver();
}

void MailserverCycle::peerSummaryChange(QVector<QString> peers)
{
	QMutexLocker locker(&m_mutex);

	// When a node is added as a peer, or disconnected
	// a DiscoverySummary signal is emitted. In here we
	// change the status of the nodes the app is connected to
	// Connected / Disconnected

	bool available = false;

	for(QHash<QString, MailserverStatus>::const_iterator it = nodes.cbegin(), end = nodes.cend(); it != end; ++it)
	{
		if(!peers.contains(it.key()) && (nodes[it.key()] == MailserverStatus::Connected || nodes[it.key()] == MailserverStatus::Trusted))
		{
			//qDebug() << "Peer disconnected: " << it.key();

			nodes[it.key()] = MailserverStatus::Disconnected;
			if(get_activeMailserver() == it.key())
			{
				qWarning() << "Active mailserver disconnected! " << it.key();
				update_activeMailserver("");
			}
		}
	}

	for(QString& peer : peers)
	{
		if(nodes.contains(peer) && (nodes[peer] == MailserverStatus::Connected || nodes[peer] == MailserverStatus::Trusted))
			continue;

		// qDebug() << "Peer connected: " << peer;
		nodes[peer] = MailserverStatus::Connected;

		if(peer == get_activeMailserver())
		{
			if(nodes.contains(peer))
			{
				trustPeer(peer);
				available = true;
			}
			updateMailserver(peer);
		}
	}

	if(available)
	{
		qDebug() << "Mailserver available!";
		emit mailserverAvailable();
	}
}