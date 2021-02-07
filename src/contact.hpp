#pragma once

#include "message.hpp"
#include <QDebug>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

class Contact : public QObject
{
	Q_OBJECT

public:
	explicit Contact(QString id, QObject* parent = nullptr);
	explicit Contact(Message* msg, QObject* parent = nullptr);
	explicit Contact(const QJsonValue data, QObject* parent = nullptr);
	virtual ~Contact();

	QML_READONLY_PROPERTY(QString, id)
	QML_READONLY_PROPERTY(QString, address)
	QML_READONLY_PROPERTY(QString, name)
	QML_READONLY_PROPERTY(bool, ensVerified)
	QML_READONLY_PROPERTY(QString, ensVerifiedAt)
	QML_READONLY_PROPERTY(QString, lastENSClockValue)
	QML_READONLY_PROPERTY(QString, ensVerificationRetries)
	QML_READONLY_PROPERTY(QString, alias)
	QML_READONLY_PROPERTY(QString, identicon)
	QML_READONLY_PROPERTY(QString, lastUpdated)
	QML_READONLY_PROPERTY(QVector<QString>, systemTags)
	// TODO: DeviceInfo ???
	QML_READONLY_PROPERTY(QString, tributeToTalk)
	QML_READONLY_PROPERTY(QString, localNickname)
	// TODO: ???? QML_READONLY_PROPERTY(QVector<QString>, images)

private:
	QMutex m_mutex;

public:
	bool operator==(const Contact& m);

	void update(const QJsonValue data);
	
	Q_INVOKABLE void save();
	Q_INVOKABLE void changeNickname(QString newNickname);
};
