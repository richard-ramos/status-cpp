#pragma once

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

class StickerPack : public QObject
{
	Q_OBJECT

public:
	explicit StickerPack(int id,
						 QStringList category,
						 QString address,
						 bool mintable,
						 QString timestamp,
						 QString price,
						 QString contentHash,
						 QObject* parent = nullptr);

	void loadContent(QNetworkAccessManager* manager);

	QML_READONLY_PROPERTY(int, id)
	QML_READONLY_PROPERTY(QStringList, category)
	QML_READONLY_PROPERTY(QString, address)
	QML_READONLY_PROPERTY(bool, mintable)
	QML_READONLY_PROPERTY(QString, timestamp)
	QML_READONLY_PROPERTY(QString, price)
	QML_READONLY_PROPERTY(QString, contentHash)
	QML_READONLY_PROPERTY(QString, name)
	QML_READONLY_PROPERTY(QString, author)
	QML_READONLY_PROPERTY(QString, preview)
	QML_READONLY_PROPERTY(QString, thumbnail)
	QML_READONLY_PROPERTY(QStringList, stickers);
};
