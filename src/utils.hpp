#pragma once

#include <QString>
#include <QJsonObject>
#include <QVector>
#include <QJsonArray>


class Utils
{
public:
	static QString generateAlias(QString publicKey);
	static QString generateIdenticon(QString publicKey);
	static void copyToClipboard(const QString& value);

	static QString jsonToStr(QJsonObject& obj);
	static QString jsonToStr(QJsonArray& arr);
	static QJsonArray toJsonArray(const QVector<QString>& value);
	static QVector<QString> toStringVector(const QJsonArray& arr);
};
