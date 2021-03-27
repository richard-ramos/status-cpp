#include "price-watcher.hpp"
#include "settings.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>

PriceWatcher::PriceWatcher(QObject* parent)
	: QObject(parent)
{
	manager = new QNetworkAccessManager(); // TODO have a single QNetworkAccessManager in the app
	QObject::connect(manager, &QNetworkAccessManager::finished, this, &PriceWatcher::onQueryFinish);
}

PriceWatcher::~PriceWatcher()
{
	qDebug() << "Stopping price watcher";
}

void PriceWatcher::fetch()
{
	qDebug() << "Fetching prices";
	
	QString currency = Settings::instance()->currency();
	QStringList visibleTokens = QStringList(Settings::instance()->visibleTokens().toList());

	QUrl url = QUrl("https://min-api.cryptocompare.com/data/pricemultifull?fsyms=ETH,SNT," + visibleTokens.join(",") + "&tsyms=" + currency +
					"&extraParams=Status.im");

	QNetworkRequest request(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
	manager->get(request);
}

void PriceWatcher::onQueryFinish(QNetworkReply* reply)
{
	QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> networkReply(reply);

	if(networkReply->error() != QNetworkReply::NoError)
	{
		qCritical() << networkReply->error();
		return;
	}

	const QJsonObject prices = QJsonDocument::fromJson(networkReply->readAll()).object();

	if(prices["RAW"].isUndefined())
	{
		qCritical() << "No prices available";
		qCritical() << prices;
		return;
	}

	const QJsonObject rawPrices = prices["RAW"].toObject();

	QMap<QString, double> priceMap;
	QString currency;
	foreach(const QString& symbol, rawPrices.keys())
	{
		const QJsonObject symbolPriceObj = rawPrices[symbol].toObject();
		currency = symbolPriceObj.keys()[0];
		priceMap[symbol] = symbolPriceObj[currency]["PRICE"].toDouble();
	}

	emit priceUpdated(currency, priceMap);
}