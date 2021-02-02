#include "chat.hpp"
#include "status.hpp"
#include <QDateTime>
#include "utils.hpp"
#include <QDebug>
#include <QVariant>
#include <QRandomGenerator>
#include <stdexcept>
#include <QJsonObject>
#include "status.hpp"
#include <QJsonArray>
#include "chat-type.hpp"
#include "settings.hpp"
#include <QtConcurrent>
#include <QFutureWatcher>
#include "messages-model.hpp"

Chat::Chat(QString id, ChatType chatType, QObject* parent, QString name, QString profile, QString color, bool active, QString timestamp, QString lastClockValue, QString deletedAtClockValue, int unviewedMessagesCount, bool muted) :
    QObject(parent),
    m_id(id),
    m_chatType(chatType),
    m_name(name),
    m_profile(profile),
    m_color(color),
    m_active(active),
    m_timestamp(timestamp),
    m_lastClockValue(lastClockValue),
    m_deletedAtClockValue(deletedAtClockValue),
    m_unviewedMessagesCount(unviewedMessagesCount),
    m_muted(muted)
{
    // Needs to be initialized because it's undefined
    m_messages = new MessagesModel();
}

Chat::Chat(const QJsonValue data, QObject * parent) :
    QObject(parent),
    m_id(data["id"].toString()),
    m_name(data["name"].toString()),
    m_profile(data["profile"].toString()),
    m_color(data["color"].toString()),
    m_active(data["active"].toBool()),
    m_timestamp(data["timestamp"].toString()),
    m_lastClockValue(data["lastClockValue"].toString()),
    m_deletedAtClockValue(data["deletedAtClockValue"].toString()),
    m_unviewedMessagesCount(data["unviewedMessagesCount"].toInt()),
    m_muted(data["muted"].toBool())
{
    int chatType = data["chatType"].toInt();
    if (chatType < ChatType::Unknown || chatType > ChatType::ComunityChat) {
        m_chatType = ChatType::Unknown;
    }
    else {
        m_chatType = static_cast<ChatType>(chatType);
    }

    // Needs to be initialized because it's undefined
    m_messages = new MessagesModel();
}


void Chat::update(const QJsonValue data){
    update_name(data["name"].toString());
    update_timestamp(data["timestamp"].toString());
    update_lastClockValue(data["lastClockValue"].toString());
    update_deletedAtClockValue(data["deletedAtClockValue"].toString());
    update_unviewedMessagesCount(data["unviewedMessagesCount"].toInt());
    update_muted(data["muted"].toBool());
}


void Chat::sendMessage(QString message, bool isReply, bool isEmoji) {
    QString preferredUsername = Settings::instance()->preferredName();
    QtConcurrent::run([=] {
        QJsonObject obj{
            {"chatId", m_id},
            {"text", message},
            // TODO: {"responseTo", replyTo},
            {"ensName", preferredUsername},
            {"sticker", QJsonValue()},
            {"contentType", 1} // TODO: replace by enum class
           // TODO: {"communityId", communityId}
        };
        const auto response = Status::instance()->callPrivateRPC("wakuext_sendChatMessage", QJsonArray{ obj }.toVariantList()).toJsonObject();
        if (!response["error"].isUndefined()) {
            throw std::domain_error(response["error"]["message"].toString().toUtf8());
        }
        });
}


void Chat::loadFilter()
{
    QtConcurrent::run([=] {
        QJsonObject obj{
            {"ChatID", m_id},
            {"OneToOne", m_chatType == ChatType::OneToOne}
        };
        const auto response = Status::instance()->callPrivateRPC("wakuext_loadFilters", QJsonArray{ QJsonArray{obj} }.toVariantList()).toJsonObject();
        if (!response["error"].isUndefined()) {
            throw std::domain_error(response["error"]["message"].toString().toUtf8());
        }
        });
}


void Chat::save()
{
    m_timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());

    if (m_chatType == ChatType::Public) {
        m_name = m_id;
    }

    if (m_color == "") {
        const QString accountColors[7]{ "#9B832F", "#D37EF4", "#1D806F", "#FA6565", "#7CDA00", "#887af9", "#8B3131" };
        m_color = accountColors[QRandomGenerator::global()->bounded(7)];
    }

    QJsonObject chat{
        {"id", m_id},
        {"name", m_name},
        {"lastClockValue", m_lastClockValue},
        {"color", m_color},
        {"lastMessage", QJsonValue()},
        {"active", m_active},
        {"profile", m_profile},
        {"unviewedMessagesCount", m_unviewedMessagesCount},
        {"chatType",  m_chatType},
        {"timestamp", m_timestamp}
    };

    const auto response = Status::instance()->callPrivateRPC("wakuext_saveChat", QJsonArray{ chat }.toVariantList()).toJsonObject();
    if (!response["error"].isUndefined()) {
        throw std::domain_error(response["error"]["message"].toString().toUtf8());
    }
}