#pragma once

#include <QHash>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlHelpers>
#include <QReadWriteLock>
#include <QString>
#include <QThread>
#include <QVector>

class PriceWatcher : public QObject
{
	Q_OBJECT

public:
	PriceWatcher(QObject* parent = nullptr);
	~PriceWatcher();

	Q_INVOKABLE void fetch();
	Q_INVOKABLE void onQueryFinish(QNetworkReply* reply);

private:
	mutable QReadWriteLock lock;
	QNetworkAccessManager* manager;
	QNetworkReply* reply;

signals:
	void priceUpdated(QString currency, QMap<QString, double> priceMap);

};