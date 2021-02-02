#include "utils.hpp"
#include "libstatus.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

QString Utils::generateAlias(QString publicKey)
{
	return QString(GenerateAlias(publicKey.toUtf8().data()));
}

QString Utils::generateIdenticon(QString publicKey)
{
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
