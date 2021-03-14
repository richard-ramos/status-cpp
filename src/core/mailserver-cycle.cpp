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

// How mailserver should work ?
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
{
	QObject::connect(this, &MailserverCycle::mailserverAvailable, this, &MailserverCycle::initialMailserverRequest);
}

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

void MailserverCycle::updateMailserver(QString enode)
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_updateMailservers", QJsonArray{QJsonArray{enode}}.toVariantList());
}

void MailserverCycle::connect(QString enode)
{
	qDebug() << "Connecting to " << enode;

	bool mailserverConnected = false;

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
		updateMailserver(enode);
		mailserverConnected = true;
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

	if(mailserverConnected)
	{
		qDebug() << "Mailserver Available!";
		emit mailserverAvailable();
	}
}

void MailserverCycle::timeoutConnection(QString enode)
{
	QMutexLocker locker(&m_mutex);
	if(nodes[enode] != MailserverStatus::Connecting) return;

	qDebug() << "Connection attempt failed due to timeout";
	nodes[enode] == MailserverStatus::Disconnected;
	if(get_activeMailserver() == enode) update_activeMailserver("");
}

int poolSize(int fleetSize)
{
	return static_cast<int>(std::ceil(fleetSize / 4.0));
}

QVector<QString> MailserverCycle::getMailservers()
{
	return Utils::toStringVector(Settings::instance()->getNodeConfig()["ClusterConfig"].toObject()["TrustedMailServers"].toArray());
	// TODO: get custom mailservers
}

void MailserverCycle::findNewMailserver()
{
	qDebug() << "Finding a new mailserver";

	const auto pingResponse =
		Status::instance()
			->callPrivateRPC("mailservers_ping",
							 QJsonArray{QJsonObject{{"addresses", Utils::toJsonArray(getMailservers())}, {"timeoutMs", 500}}}.toVariantList())
			.toJsonObject();

	QVector<std::tuple<QString, int>> availableMailservers;
	foreach(const QJsonValue& value, pingResponse["result"].toArray())
	{
		if(!value["error"].isNull()) continue;
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
	qDebug() << "Mailserver pool: " << poolN << " - Available mailservers: " << availableMailservers.count();
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

	if(nodes.contains(get_activeMailserver()) && nodes[get_activeMailserver()] == MailserverStatus::Connected)
	{
		qDebug() << "Mailserver is already connected. Skipping iteration";
		return;
	}

	qDebug() << "Automatically switching mailserver";

	if(get_activeMailserver() != "") disconnectActiveMailserver();

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
		if(!peers.contains(it.key()) && nodes[it.key()] == MailserverStatus::Connected)
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
		if(nodes.contains(peer) && nodes[peer] == MailserverStatus::Connected) continue;

		// qDebug() << "Peer connected: " << peer;
		nodes[peer] = MailserverStatus::Connected;

		if(peer == get_activeMailserver())
		{
			if(nodes.contains(peer))
			{
				updateMailserver(peer);
				available = true;
			}
		}
	}

	if(available)
	{
		qDebug() << "PeerSummaryChange - Mailserver available!";
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

std::optional<QVector<Topic>> MailserverCycle::getMailserverTopicByChatId(QString chatId, bool isOneToOne)
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_filters", QJsonArray{}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		qCritical() << "Couldn't load filters" << response["error"];
		return {};
	}

	QVector<Topic> results;
	QStringList topicList;
	foreach(const QJsonValue& value, response["result"].toArray())
	{
		const QJsonObject obj = value.toObject();

		if(!(obj["chatId"].toString() == chatId || (obj["identity"].toString() == chatId && obj["oneToOne"].toBool() == true))) continue;
		topicList << obj["topic"].toString();
	}

	QVector<Topic> topics = getMailserverTopics();
	foreach(const Topic& topic, topics)
	{
		if(topicList.contains(topic.topic))
		{
			results << topic;
		}
	}
	if(results.size() > 0)
	{
		return results;
	}

	return {};
}

void MailserverCycle::removeMailserverTopicForChat(QString chatId)
{
	QVector<Topic> topics = getMailserverTopics();
	QVector<Topic> topicsToUpdate;
	foreach(Topic t, topics)
	{
		if(t.chatIds.contains(chatId))
		{
			if(t.chatIds.count() > 1)
			{
				t.chatIds.remove(t.chatIds.indexOf(chatId));
				topicsToUpdate << t;
			}
			else
			{
				const auto response =
					Status::instance()->callPrivateRPC("mailservers_deleteMailserverTopic", QJsonArray{t.topic}.toVariantList()).toJsonObject();
			}
		}
	}

	if(topicsToUpdate.count() == 0) return;
	foreach(Topic t, topicsToUpdate)
	{
		addMailserverTopic(t);
	}
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
	qDebug() << "Requesting messages to " << get_activeMailserver() << fromValue << toValue;
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
	});
}

void MailserverCycle::initialMailserverRequest()
{
	qDebug() << "Initial request";

	QObject::disconnect(this, &MailserverCycle::mailserverAvailable, this, &MailserverCycle::initialMailserverRequest);

	QVector<Topic> mailserverTopics = getMailserverTopics();

	qint64 fromValue = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() - 86400; // 24 hours ago

	if(mailserverTopics.count() == 0) return;

	// TODO: how to do a map and min?
	int minRequest = mailserverTopics.at(0).lastRequest;
	QVector<Topic> topicsToRequest;
	for(int i = 0; i < mailserverTopics.size(); ++i)
	{
		if(mailserverTopics.at(i).lastRequest > fromValue) continue;

		topicsToRequest << mailserverTopics.at(i);
		if(mailserverTopics.at(i).lastRequest < minRequest)
		{
			minRequest = mailserverTopics.at(i).lastRequest;
		}
	}

	if(minRequest < fromValue)
	{
		minRequest = fromValue; // Only last 24hrs
	}

	QVector<QString> topicList;
	foreach(Topic t, topicsToRequest)
	{
		topicList << t.topic;
	}

	if(topicList.size() == 0) return;

	if(!isMailserverAvailable()) return; // TODO: add a pending request

	// Updating topic request date
	foreach(Topic t, topicsToRequest)
	{
		t.lastRequest = minRequest;
		addMailserverTopic(t);
	}

	requestMessages(topicList, minRequest);
	emit requestSent();
}

void MailserverCycle::requestMessages(QString chatId, bool isOneToOne, int earliestKnownMessageTimestamp)
{
	if(!isMailserverAvailable()) return; // TODO: add a pending request

	auto topics = getMailserverTopicByChatId(chatId, isOneToOne);
	if(!topics.has_value()) return;

	QVector<Topic> topicVector = topics.value();
	qint64 from = earliestKnownMessageTimestamp - 86400;
	QVector<QString> topicsToRequest;

	foreach(const Topic& t, topicVector)
	{
		if(t.lastRequest - 86400 > from)
		{
			// Get the more recent date from the list of topics
			from = t.lastRequest - 86400;
		}
		topicsToRequest << t.topic;
	}

	if(from < 0) from = 0;

	if(!isMailserverAvailable()) return; // TODO: add a pending request

	// Updating topic request date
	foreach(Topic t, topicVector)
	{
		t.lastRequest = from;
		addMailserverTopic(t);
	}

	requestMessages(topicsToRequest, from, earliestKnownMessageTimestamp);
	emit requestSent();
}

void MailserverCycle::requestMessagesInLast(QString chatId, bool isOneToOne, int fetchRange)
{
	if(!isMailserverAvailable()) return; // TODO: add a pending request

	auto topics = getMailserverTopicByChatId(chatId, isOneToOne);
	if(!topics.has_value()) return;

	QVector<Topic> topicVector = topics.value();
	qint64 from = QDateTime::currentSecsSinceEpoch() - fetchRange;
	QVector<QString> topicsToRequest;

	foreach(const Topic& t, topicVector)
	{
		topicsToRequest << t.topic;
	}

	if(!isMailserverAvailable()) return; // TODO: add a pending request

	// Updating topic request date
	foreach(Topic t, topicVector)
	{
		if(t.lastRequest > from)
		{
			t.lastRequest = from;
			addMailserverTopic(t);
		}
	}

	requestMessages(topicsToRequest, from);
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

	if(!isMailserverAvailable()) return; // TODO: add a pending request

	requestMessages(topicList);
	emit requestSent();
}

bool MailserverCycle::isMailserverAvailable()
{
	// TODO: does this require a mutex?  consider replace this for read locker
	return m_activeMailserver != "" && nodes.contains(m_activeMailserver) && nodes[m_activeMailserver] == MailserverStatus::Connected;
}