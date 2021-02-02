#pragma once

#include <QString>
#include <QJsonObject>


class Utils
{
public:
	static QString generateAlias(QString publicKey);
	static QString generateIdenticon(QString publicKey);

	static QString jsonToStr(QJsonObject& obj);
	static QString jsonToStr(QJsonArray& arr);
};
