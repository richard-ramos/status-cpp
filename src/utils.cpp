#include "utils.hpp"
#include "libstatus.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QClipboard>
#include <QGuiApplication>

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


/*
QString Utils::resolveENS(QString ensName)
{

}*/

void Utils::copyToClipboard(const QString& value)
{
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(value);

}


