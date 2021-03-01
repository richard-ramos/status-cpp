#include "utils.hpp"
#include "libstatus.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>

QString Utils::generateAlias(QString publicKey)
{
	if(publicKey.isEmpty()) return "";
	return QString(GenerateAlias(publicKey.toUtf8().data()));
}

QString Utils::generateIdenticon(QString publicKey)
{
	if(publicKey.isEmpty()) return "";
	return QString(Identicon(publicKey.toUtf8().data()));
}

QString Utils::jsonToStr(QJsonObject& obj)
{
	QJsonDocument doc(obj);
	return QString::fromUtf8(doc.toJson());
}

QString Utils::jsonToStr(QJsonArray& arr)
{
	QJsonDocument doc(arr);
	return QString::fromUtf8(doc.toJson());
}

QJsonArray Utils::toJsonArray(const QVector<QString>& value)
{
	QJsonArray array;
	for(auto& v : value)
		array << v;
	return array;
}

void Utils::copyToClipboard(const QString& value)
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	clipboard->setText(value);
}

QVector<QString> Utils::toStringVector(const QJsonArray& arr)
{
	QVector<QString> result;
	foreach(const QJsonValue& value, arr)
	{
		result << value.toString();
	}
	return result;
}
