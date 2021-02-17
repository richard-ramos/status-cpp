#pragma once

#include <QDebug>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>


struct ContactImage {
	QString type;
	QString uri;
	int fileSize;
	int height;
	QString keyUid;
	int resizeTarget;
	int width;
};

class Contact : public QObject
{
	Q_OBJECT

public:
	explicit Contact(QString id, QObject* parent = nullptr);
	explicit Contact(QString id, QString ensName, QObject* parent = nullptr);
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

	Q_PROPERTY(bool isAdded READ isAdded NOTIFY contactToggled)
	Q_PROPERTY(bool isBlocked READ isBlocked NOTIFY blockedToggled)
	Q_PROPERTY(QString image READ image NOTIFY imageChanged)


signals:
	void contactToggled(QString contactId);
	void blockedToggled(QString contactId);
	void imageChanged(QString contactId);

private:
	QMutex m_mutex;
	QVector<QString> m_systemTags;
	QVector<ContactImage> m_images;


public:
	bool operator==(const Contact& m);

	void update(const QJsonValue data);
	void update(Contact* contact);

	bool isAdded();
	bool isBlocked();
	QString image();

	Q_INVOKABLE void save();
	Q_INVOKABLE void changeNickname(QString newNickname);
	Q_INVOKABLE void toggleAdd();
	Q_INVOKABLE void toggleBlock();

};
