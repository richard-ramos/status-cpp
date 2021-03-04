#pragma once

#include <QJSEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVector>

class Utils : public QObject
{
	Q_OBJECT

public:
	explicit Utils(QObject* parent = nullptr);

public:
	Q_INVOKABLE static QString generateAlias(QString publicKey);
	Q_INVOKABLE static QString generateIdenticon(QString publicKey);
	Q_INVOKABLE static void copyToClipboard(const QString& value);

	Q_INVOKABLE static QString generateQRCode(QString publicKey);
	Q_INVOKABLE static QString plainText(const QString& value);


	static QString jsonToStr(QJsonObject& obj);
	static QString jsonToStr(QJsonArray& arr);
	static QJsonArray toJsonArray(const QVector<QString>& value);
	static QVector<QString> toStringVector(const QJsonArray& arr);
};

static QObject* utilsProvider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
	return new Utils();
}