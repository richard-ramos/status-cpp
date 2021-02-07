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
	// TODO: DeviceInfo ???
	QML_READONLY_PROPERTY(QString, tributeToTalk)
	QML_READONLY_PROPERTY(QString, localNickname)
	// TODO: ???? QML_READONLY_PROPERTY(QVector<QString>, images)

	Q_PROPERTY(bool isAdded READ isAdded NOTIFY contactToggled)
	Q_PROPERTY(bool isBlocked READ isBlocked NOTIFY blockedToggled)


signals:
	void contactToggled(QString chatId);
	void blockedToggled(QString chatId);

private:
	QMutex m_mutex;
	QVector<QString> m_systemTags;

public:
	bool operator==(const Contact& m);

	void update(const QJsonValue data);
	bool isAdded();
	bool isBlocked();

	Q_INVOKABLE void save();
	Q_INVOKABLE void changeNickname(QString newNickname);
	Q_INVOKABLE void toggleAdd();
	Q_INVOKABLE void toggleBlock();

};
