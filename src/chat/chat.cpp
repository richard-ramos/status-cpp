#include "chat.hpp"
#include "chat-type.hpp"
#include "constants.hpp"
#include "content-type.hpp"
#include "mailserver-cycle.hpp"
#include "messages-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "utils.hpp"
#include <QColorSpace>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QQmlApplicationEngine>
#include <QRandomGenerator>
#include <QString>
#include <QVariant>
#include <QtConcurrent>
#include <stdexcept>

Chat::Chat(QObject* parent,
		   QString id,
		   ChatType chatType,
		   QString name,
		   QString profile,
		   QString color,
		   bool active,
		   QString timestamp,
		   QString lastClockValue,
		   QString deletedAtClockValue,
		   int unviewedMessagesCount,
		   bool muted)
	: QObject(parent)
	, m_id(id)
	, m_chatType(chatType)
	, m_name(name)
	, m_profile(profile)
	, m_color(color)
	, m_active(active)
	, m_timestamp(timestamp)
	, m_lastClockValue(lastClockValue)
	, m_deletedAtClockValue(deletedAtClockValue)
	, m_unviewedMessagesCount(unviewedMessagesCount)
	, m_muted(muted)
{
	// Needs to be initialized because it's undefined
	m_messages = new MessagesModel(m_id, chatType);
	m_messages->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_messages, QQmlApplicationEngine::CppOwnership);

	m_lastMessage = new Message();
	m_lastMessage->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_lastMessage, QQmlApplicationEngine::CppOwnership);
}

Chat::~Chat() { }

bool Chat::operator==(const Chat& c)
{
	return m_id == c.get_id();
}

Chat::Chat(QObject* parent, const QJsonValue data)
	: QObject(parent)
	, m_id(data["id"].toString())
	, m_name(data["name"].toString())
	, m_profile(data["profile"].toString())
	, m_color(data["color"].toString())
	, m_active(data["active"].toBool())
	, m_timestamp(data["timestamp"].toString())
	, m_lastClockValue(data["lastClockValue"].toString())
	, m_deletedAtClockValue(data["deletedAtClockValue"].toString())
	, m_unviewedMessagesCount(data["unviewedMessagesCount"].toInt())
	, m_muted(data["muted"].toBool())
	, m_identicon(data["identicon"].toString())
{
	int chatType = data["chatType"].toInt();
	if(chatType < ChatType::Unknown || chatType > ChatType::ComunityChat)
	{
		m_chatType = ChatType::Unknown;
	}
	else
	{
		m_chatType = static_cast<ChatType>(chatType);
	}

	// Needs to be initialized because it's undefined
	m_messages = new MessagesModel(m_id, m_chatType);
	m_messages->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_messages, QQmlApplicationEngine::CppOwnership);

	m_lastMessage = new Message(data["lastMessage"]);
	m_lastMessage->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_lastMessage, QQmlApplicationEngine::CppOwnership);

	m_hasMentions = false;
	if(m_lastMessage->get_hasMention())
	{
		m_hasMentions = true;
	}

	if(m_chatType == ChatType::PrivateGroupChat)
	{
		foreach(const QJsonValue& value, data["members"].toArray())
		{
			const QJsonObject obj = value.toObject();
			m_members << ChatMember{.admin = obj["admin"].toBool(), .id = obj["id"].toString(), .joined = obj["joined"].toBool()};
		}

		foreach(const QJsonValue& value, data["membershipUpdateEvents"].toArray())
		{
			const QJsonObject obj = value.toObject();
			ChatMembershipEvent c;
			c.chatId = obj["id"].toString();
			c.clockValue = obj["clockValue"].toString();
			c.from = obj["from"].toString();
			c.name = obj["name"].toString();
			c.rawPayload = obj["rawPayload"].toString();
			c.signature = obj["signature"].toString();
			c.type = obj["type"].toInt();
			m_membershipUpdateEvents << c;
		}
	}
}

void Chat::update(const QJsonValue data)
{
	// TODO: replace by update instead of creating a new Message
	delete m_lastMessage;
	m_lastMessage = new Message(data["lastMessage"]);

	m_lastMessage->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_lastMessage, QQmlApplicationEngine::CppOwnership);

	update_name(data["name"].toString());
	update_timestamp(data["timestamp"].toString());
	update_lastClockValue(data["lastClockValue"].toString());
	update_deletedAtClockValue(data["deletedAtClockValue"].toString());
	update_unviewedMessagesCount(data["unviewedMessagesCount"].toInt());
	update_muted(data["muted"].toBool());

	if(m_chatType == ChatType::PrivateGroupChat)
	{
		m_members.clear();
		foreach(const QJsonValue& value, data["members"].toArray())
		{
			const QJsonObject obj = value.toObject();
			ChatMember c;
			c.id = obj["id"].toString();
			c.joined = obj["joined"].toBool();
			c.admin = obj["admin"].toBool();
			m_members << c;
		}

		m_membershipUpdateEvents.clear();
		foreach(const QJsonValue& value, data["membershipUpdateEvents"].toArray())
		{
			const QJsonObject obj = value.toObject();
			ChatMembershipEvent c;
			c.chatId = obj["id"].toString();
			c.clockValue = obj["clockValue"].toString();
			c.from = obj["from"].toString();
			c.name = obj["name"].toString();
			c.rawPayload = obj["rawPayload"].toString();
			c.signature = obj["signature"].toString();
			c.type = obj["type"].toInt();
			m_membershipUpdateEvents << c;
		}

		emit groupDataChanged();
	}
}

void Chat::sendMessage(QString message, QString replyTo, bool isEmoji)
{
	QString preferredUsername = Settings::instance()->preferredName();
	emit sendingMessage();

	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);
		QJsonObject obj{
			{"chatId", m_id},
			{"text", message},
			{"responseTo", replyTo},
			{"ensName", preferredUsername},
			{"sticker", QJsonValue()},
			{"contentType", isEmoji ? ContentType::Emoji : ContentType::Message}
			// TODO: {"communityId", communityId}
		};
		const auto response = Status::instance()->callPrivateRPC("wakuext_sendChatMessage", QJsonArray{obj}.toVariantList()).toJsonObject();
		if(!response["error"].isUndefined())
		{
			emit sendingMessageFailed();
			throw std::domain_error(response["error"]["message"].toString().toUtf8());
		}
		Status::instance()->emitMessageSignal(response["result"].toObject());
	});
}

void Chat::sendSticker(int packId, QString stickerHash)
{
	QString preferredUsername = Settings::instance()->preferredName();
	emit sendingMessage();

	Settings::instance()->addRecentSticker(packId, stickerHash);

	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);
		QJsonObject obj{
			{"chatId", m_id},
			{"text", "Update to latest version to see a nice sticker here!"},
			{"responseTo", QJsonValue()},
			{"ensName", preferredUsername},
			{"sticker", QJsonObject{{"hash", stickerHash}, {"pack", packId}}},
			{"contentType", ContentType::Sticker}
			// TODO: {"communityId", communityId}
		};
		const auto response = Status::instance()->callPrivateRPC("wakuext_sendChatMessage", QJsonArray{obj}.toVariantList()).toJsonObject();
		if(!response["error"].isUndefined())
		{
			emit sendingMessageFailed();
			throw std::domain_error(response["error"]["message"].toString().toUtf8());
		}
		Status::instance()->emitMessageSignal(response["result"].toObject());
	});
}

void Chat::sendImage(QString imagePath)
{
	QString preferredUsername = Settings::instance()->preferredName();
	emit sendingMessage();

	QImage img(QUrl(imagePath).toLocalFile());
	img.setColorSpace(QColorSpace::SRgb);
	int w = img.width();
	int h = img.height();

	QPixmap pixmap;
	pixmap = pixmap.fromImage(img.scaled(Constants::MaxImageSize < w ? Constants::MaxImageSize : w,
										 Constants::MaxImageSize < h ? Constants::MaxImageSize : h,
										 Qt::KeepAspectRatio,
										 Qt::SmoothTransformation));

	auto newFilePath = Constants::tmpPath("/" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".jpg");

	QFile file(newFilePath);
	file.open(QIODevice::WriteOnly);
	pixmap.save(&file, "jpeg", 75);
	file.close();

	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);
		QJsonObject obj{
			{"chatId", m_id},
			{"text", "Update to latest version to see a nice image here!"},
			{"imagePath", QFileInfo(newFilePath).absoluteFilePath()},
			{"ensName", preferredUsername},
			{"sticker", QJsonValue()},
			{"contentType", ContentType::Image}
			// TODO: {"communityId", communityId}
		};

		const auto response = Status::instance()->callPrivateRPC("wakuext_sendChatMessage", QJsonArray{obj}.toVariantList()).toJsonObject();
		if(!response["error"].isUndefined())
		{
			emit sendingMessageFailed();
			throw std::domain_error(response["error"]["message"].toString().toUtf8());
		}
		Status::instance()->emitMessageSignal(response["result"].toObject());
	});
}

void Chat::leave()
{
	m_active = false;

	if(m_chatType == ChatType::PrivateGroupChat)
	{
		leaveGroup();
	}

	deleteChatHistory();
	save();
	left(m_id);
}

void Chat::loadFilter()
{
	QtConcurrent::run([=] {
		QMutexLocker locker(&m_mutex);
		QJsonObject obj{{"ChatID", m_id}, {"OneToOne", m_chatType == ChatType::OneToOne}};
		const auto response = Status::instance()->callPrivateRPC("wakuext_loadFilters", QJsonArray{QJsonArray{obj}}.toVariantList()).toJsonObject();
		if(!response["error"].isUndefined())
		{
			throw std::domain_error(response["error"]["message"].toString().toUtf8());
		}

		foreach(const QJsonValue& value, response["result"].toArray())
		{
			// Handle non public chats
			if(value["chatId"].toString() == m_id || (value["identity"].toString() == m_id && value["oneToOne"].toBool() == true))
			{
				Topic t;
				t.topic = value["topic"].toString();
				t.discovery = value["discovery"].toBool();
				t.negotiated = value["negotiated"].toBool();
				t.chatIds << value["chatId"].toString();
				t.lastRequest = 1;

				emit topicCreated(t);
				break;
			}
		}
	});
}

void Chat::deleteChatHistory()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_deleteMessagesByChatID", QJsonArray{m_id}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw std::domain_error(response["error"]["message"].toString().toUtf8());
	}

	auto msg = new Message(QJsonValue{});
	msg->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(msg, QQmlApplicationEngine::CppOwnership);

	update_lastMessage(msg);
	update_unviewedMessagesCount(0);
	m_messages->clear();
}

void Chat::markAllMessagesAsRead()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_markAllRead", QJsonArray{m_id}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw std::domain_error(response["error"]["message"].toString().toUtf8());
	}

	update_unviewedMessagesCount(0);
	save();
}

void Chat::loadMoreMessages()
{
	m_messages->loadMessages(false);
	m_messages->loadReactions(false);

	emit messagesLoaded();
}

void Chat::save()
{
	//QtConcurrent::run([=] {
	//	QMutexLocker locker(&m_mutex);

	m_timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());

	if(m_chatType == ChatType::Public)
	{
		m_name = m_id;
	}

	if(m_color == "")
	{
		const QString accountColors[7]{"#9B832F", "#D37EF4", "#1D806F", "#FA6565", "#7CDA00", "#887af9", "#8B3131"};
		m_color = accountColors[QRandomGenerator::global()->bounded(7)];
	}

	QJsonObject chat{{"id", m_id},
					 {"name", m_name},
					 {"lastClockValue", m_lastClockValue},
					 {"color", m_color},
					 {"lastMessage", QJsonValue()}, // TODO: serialize last message
					 {"active", m_active},
					 {"profile", m_profile},
					 {"unviewedMessagesCount", m_unviewedMessagesCount},
					 {"chatType", m_chatType},
					 {"timestamp", m_timestamp}};

	const auto response = Status::instance()->callPrivateRPC("wakuext_saveChat", QJsonArray{chat}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw std::domain_error(response["error"]["message"].toString().toUtf8());
	}
	//});
}

uint qHash(const ChatMember& item, uint seed)
{
	return qHash(item.id, seed);
}

QSet<ChatMember> Chat::getChatMembers()
{
	return m_members;
}

void Chat::renameGroup(QString newName)
{
	const auto response =
		Status::instance()->callPrivateRPC("wakuext_changeGroupChatName", QJsonArray{QJsonValue(), m_id, newName}.toVariantList()).toJsonObject();

	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());
}

void Chat::makeAdmin(QString memberId)
{
	const auto response = Status::instance()
							  ->callPrivateRPC("wakuext_addAdminsToGroupChat", QJsonArray{QJsonValue(), m_id, QJsonArray{memberId}}.toVariantList())
							  .toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());

	emit groupDataChanged();
}

void Chat::removeFromGroup(QString memberId)
{
	const auto response = Status::instance()
							  ->callPrivateRPC("wakuext_removeMemberFromGroupChat", QJsonArray{QJsonValue(), m_id, memberId}.toVariantList())
							  .toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());

	emit groupDataChanged();
}

void Chat::addMembers(QStringList members)
{
	const auto response =
		Status::instance()
			->callPrivateRPC("wakuext_addMembersToGroupChat", QJsonArray{QJsonValue(), m_id, QJsonArray::fromStringList(members)}.toVariantList())
			.toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());

	emit groupDataChanged();
}

void Chat::join()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_confirmJoiningGroup", QJsonArray{m_id}.toVariantList()).toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());
	emit groupDataChanged();
}

void Chat::leaveGroup()
{
	const auto response =
		Status::instance()->callPrivateRPC("wakuext_leaveGroupChat", QJsonArray{QJsonValue(), m_id, true}.toVariantList()).toJsonObject();
	// TODO: error handling
	Status::instance()->emitMessageSignal(response["result"].toObject());
}

void Chat::requestMoreMessages(qint64 from){
	m_mailservers->getCycle()->requestMessages(m_id, m_chatType == ChatType::OneToOne, static_cast<int>(from/1000));
	m_messages->update_oldestMsgTimestamp(from - (86400*1000));
}

void Chat::requestMessagesInLast(int fetchRange){
	m_mailservers->getCycle()->requestMessagesInLast(m_id, m_chatType == ChatType::OneToOne, fetchRange);
}