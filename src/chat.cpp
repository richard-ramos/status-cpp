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

Chat::Chat(QString id, ChatType chatType, QString name, QString profile, QString color, bool active, int timestamp, int lastClockValue, int deletedAtClockValue, int unviewedMessagesCount, bool muted, QObject * parent):
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
}

Chat::Chat(const QJsonValue data, QObject * parent): 
    QObject(parent),
    m_id(data["id"].toString()),
    m_chatType(static_cast<ChatType>(data["chatType"].toInt())),
    m_name(data["name"].toString()),
    m_profile(data["profile"].toString()),
    m_color(data["color"].toString()),
    m_active(data["active"].toBool()),
    m_timestamp(data["timestamp"].toInt()),
    m_lastClockValue(data["lastClockValue"].toInt()),
    m_deletedAtClockValue(data["deletedAtClockValue"].toInt()),
    m_unviewedMessagesCount(data["unviewedMessagesCount"].toInt()),
    m_muted(data["muted"].toBool())
{  
}

void Chat::save()
{
    m_timestamp = QDateTime::currentMSecsSinceEpoch();

    if(m_chatType == ChatType::Public){
        m_name = m_id;
    }

    if(m_color == "") {
        const QString accountColors[7]{"#9B832F", "#D37EF4", "#1D806F", "#FA6565", "#7CDA00", "#887af9", "#8B3131"};
        m_color = accountColors[QRandomGenerator::global()->bounded(7)];
    }

    QJsonObject chat {
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

    const auto response = Status::instance()->callPrivateRPC("wakuext_saveChat", QJsonArray{chat}.toVariantList()).toJsonObject();
    if(!response["error"].isUndefined()){
        throw std::domain_error(response["error"]["message"].toString().toUtf8());
    }
}