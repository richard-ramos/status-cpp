#include "chat.hpp"
#include "chat-type.hpp"
#include "content-type.hpp"
#include "mailserver-cycle.hpp"
#include "constants.hpp"
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
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QColorSpace>

Chat::Chat(QString id,
		   ChatType chatType,
		   QObject* parent,
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
	m_messages = new MessagesModel(m_id);
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

void Chat::setFilterId(QString filterId)
{
	m_filterId = filterId;
}

Chat::Chat(const QJsonValue data, QObject* parent)
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
	m_messages = new MessagesModel(m_id);
	m_messages->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_messages, QQmlApplicationEngine::CppOwnership);

	m_lastMessage = new Message(data["lastMessage"]);
	m_lastMessage->setParent(this);
	QQmlApplicationEngine::setObjectOwnership(m_lastMessage, QQmlApplicationEngine::CppOwnership);
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


void Chat::sendImage(QString imagePath)
{
	QString preferredUsername = Settings::instance()->preferredName();
	emit sendingMessage();

	qDebug() << imagePath;

 	QImage img(imagePath);
    img.setColorSpace(QColorSpace::SRgb);
    int w = img.width();
    int h = img.height();

    QPixmap pixmap;
    pixmap = pixmap.fromImage(img.scaled(Constants::MaxImageSize < w ? Constants::MaxImageSize : w, Constants::MaxImageSize < h ? Constants::MaxImageSize : h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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

		qDebug() << obj;
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
	// TODO: ideally this should happen in a separate thread and notify parent to remove item from vector
	deleteChatHistory();
	removeFilter();
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
			if(value["chatId"].toString() == m_id)
			{
				m_filterId = value["filterId"].toString();

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

void Chat::removeFilter()
{
	QJsonObject obj{{"ChatID", m_id}, {"FilterID", m_filterId}};
	const auto response = Status::instance()->callPrivateRPC("wakuext_removeFilters", QJsonArray{QJsonArray{obj}}.toVariantList()).toJsonObject();

	qDebug() << response;
	if(!response["error"].isUndefined())
	{
		throw std::domain_error(response["error"]["message"].toString().toUtf8());
	}
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
