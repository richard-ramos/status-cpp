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

// How do mailserver should work ?
//
// - We send a request to the mailserver, we are only interested in the
//   messages since `last-request` up to the last seven days
//   and the last 24 hours for topics that were just joined
// - The mailserver doesn't directly respond to the request and
//   instead we start receiving messages in the filters for the requested
//   topics.
// - If the mailserver was not ready when we tried for instance to request
//   the history of a topic after joining a chat, the request will be done
//   as soon as the mailserver becomes available

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

		// TODO: try to reconnect 3 times before switching mailserver

		QtConcurrent::run([=] {
			QThread::sleep(10); // Timeout connection attempt at 10 seconds
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

QVector<Topic> MailserverCycle::getMailserverTopics()
{
	const auto response = Status::instance()->callPrivateRPC("mailservers_getMailserverTopics", QJsonArray{}.toVariantList()).toJsonObject();
	QVector<Topic> topics;
	foreach(const QJsonValue& value, response["result"].toArray())
	{
		const QJsonObject obj = value.toObject();
		Topic t;
		t.discovery = obj["discovery?"].toBool();
		t.lastRequest = obj["last-request"].toInt();
		t.negotiated = obj["negotiated?"].toBool();
		t.topic = obj["topic"].toString();
		t.chatIds = Utils::toStringVector(obj["chat-ids"].toArray());
		topics << t;
	}
	return topics;
}

void MailserverCycle::addMailserverTopic(Topic t)
{
	const auto response = Status::instance()->callPrivateRPC("mailservers_addMailserverTopic",
															 QJsonArray{QJsonObject{{"topic", t.topic},
																					{"discovery?", t.discovery},
																					{"negotiated?", t.negotiated},
																					{"chat-ids", Utils::toJsonArray(t.chatIds)},
																					{"last-request", t.lastRequest}}}
																 .toVariantList());
}

QString MailserverCycle::generateSymKeyFromPassword()
{
	// TODO: unhardcode this for non - status mailservers
	const auto response =
		Status::instance()->callPrivateRPC("waku_generateSymKeyFromPassword", QJsonArray{"status-offline-inbox"}.toVariantList()).toJsonObject();

	return response["result"].toString();
}

void MailserverCycle::requestMessages(QVector<QString> topicList, qint64 fromValue, qint64 toValue, bool force)
{
	qDebug() << "Requesting messages from: " << get_activeMailserver();
	QString generatedSymKey = generateSymKeyFromPassword();
	requestMessagesCall(topicList, generatedSymKey, get_activeMailserver(), 1000, fromValue, toValue, force);
}

void MailserverCycle::requestMessagesCall(
	QVector<QString> topics, QString symKeyID, QString peer, int numberOfMessages, qint64 fromTimestamp, qint64 toTimestamp, bool force)
{
	QtConcurrent::run([=] {
		qint64 toValue = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
		qint64 fromValue = toValue - 86400;

		if(fromTimestamp != 0)
		{
			fromValue = fromTimestamp;
		}

		if(toTimestamp != 0)
		{
			toValue = toTimestamp;
		}

		const auto response = Status::instance()
								  ->callPrivateRPC("wakuext_requestMessages",
												   QJsonArray{QJsonObject{{"topics", Utils::toJsonArray(topics)},
																		  {"mailServerPeer", peer},
																		  {"symKeyID", symKeyID},
																		  {"timeout", 30},
																		  {"limit", numberOfMessages},
																		  {"cursor", QJsonValue()},
																		  {"from", fromValue},
																		  {"to", toValue},
																		  {"force", force}}}
													   .toVariantList())
								  .toJsonObject();

		qDebug() << response;
	});
}

void MailserverCycle::initialMailserverRequest()
{
	QMutexLocker locker(&m_mutex);

	QVector<Topic> mailserverTopics = getMailserverTopics();

	qint64 fromValue = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() - 86400; // 24 hours ago

	if(mailserverTopics.count() == 0)
		return;

	// TODO: how to do a map and min?
	int minRequest = mailserverTopics.at(0).lastRequest;
	QVector<QString> topicList;
	for(int i = 0; i < mailserverTopics.size(); ++i)
	{
		topicList << mailserverTopics.at(i).topic;
		if(mailserverTopics.at(i).lastRequest < minRequest)
		{
			minRequest = mailserverTopics.at(i).lastRequest;
		}
	}

	if(!isMailserverAvailable())
		return; // TODO: add a pending request

	requestMessages(topicList, minRequest);
	emit requestSent();
}

void MailserverCycle::addChannelTopic(Topic t)
{
	QMutexLocker locker(&m_mutex);

	QVector<Topic> topics = getMailserverTopics();
	bool found = false;
	for(int i = 0; i < topics.size(); ++i)
	{
		if(topics.at(i).topic == t.topic)
		{
			found = true;
			if(!topics.at(i).chatIds.contains(t.chatIds[0]))
			{
				// Topic exist but chat Id is not contained in topic
				auto existingTopic = topics[i];
				existingTopic.chatIds << t.chatIds[0];
				addMailserverTopic(existingTopic);
			}
			break;
		}
	}

	if(!found)
	{
		// Topic does not exist
		addMailserverTopic(t);
	}

	QVector<QString> topicList;
	topicList << t.topic;

	if(!isMailserverAvailable())
		return; // TODO: add a pending request

	requestMessages(topicList);
	emit requestSent();
}

bool MailserverCycle::isMailserverAvailable()
{
	return m_activeMailserver != "" && nodes.contains(m_activeMailserver) && nodes[m_activeMailserver] == MailserverStatus::Trusted;
}
