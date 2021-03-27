#include "balance-watcher.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "token-model.hpp"
#include "uint256_t.h"
#include "utils.hpp"
#include <QDebug>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QtConcurrent>

BalanceWatcher::BalanceWatcher(TokenModel* tokens, QObject* parent)
	: QObject(parent)
	, m_tokens(tokens)
{ }

BalanceWatcher::~BalanceWatcher()
{
	qDebug() << "Stopping balance watcher";
}

void BalanceWatcher::fetch()
{
	qDebug() << "Fetching balances";

	auto* watcher = new QFutureWatcher<QVector<AccountBalance>>(this);
	QObject::connect(watcher, &QFutureWatcher<QVector<AccountBalance>>::finished, this, [this, watcher]() {
		QVector<AccountBalance> result = watcher->result();
		foreach(AccountBalance b, result)
		{
			emit balanceFetched(b.address, b.balances);
		}
		watcher->deleteLater();
	});
	watcher->setFuture(QtConcurrent::run(this, &BalanceWatcher::queryBalances));
}

QVector<AccountBalance> BalanceWatcher::queryBalances()
{
	QVector<AccountBalance> accountBalances;

	QStringList visibleTokens = QStringList(Settings::instance()->visibleTokens().toList());
	const auto response = Status::instance()->callPrivateRPC("accounts_getAccounts", QJsonArray{}.toVariantList()).toJsonObject();
	foreach(QJsonValue accountJson, response["result"].toArray())
	{
		const QJsonObject accountObj = accountJson.toObject();
		if(accountObj["chat"].toBool() == true) continue;

		QString address = accountObj["address"].toString();
		QMap<QString, QString> balances;
		balances["ETH"] = Utils::wei2Token(QString::fromStdString(uint256_t(getETHBalance(address).toStdString()).str()), 18);

		foreach(QString token, visibleTokens)
		{
			auto tokenData = m_tokens->token(token);
			if(!tokenData.has_value())
			{
				// TODO: error handling
				qCritical() << "NO TOKEN DATA";
				continue;
			}

			Token t = tokenData.value();
			balances[t.symbol] =
				Utils::wei2Token(QString::fromStdString(uint256_t(getTokenBalance(address, t.address).toStdString()).str()), t.decimals);
		}

		accountBalances << AccountBalance{.address = address, .balances = balances};
	}

	return accountBalances;
}

QString BalanceWatcher::getETHBalance(QString address)
{
	const auto response =
		Status::instance()->callPrivateRPC("eth_getBalance", QJsonArray{address, QStringLiteral("latest")}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw response["error"].toString();
	}
	return response["result"].toString().mid(2);
}

QString BalanceWatcher::getTokenBalance(QString address, QString tokenAddress)
{

	QString postfixedAccount = address.mid(2);
	QJsonObject tokenBalanceOf{{"to", tokenAddress}, {"from", address}, {"data", QString("0x70a08231000000000000000000000000") + postfixedAccount}};
	const auto response =
		Status::instance()->callPrivateRPC("eth_call", QJsonArray{tokenBalanceOf, QStringLiteral("latest")}.toVariantList()).toJsonObject();
	if(!response["error"].isUndefined())
	{
		throw response["error"].toString();
	}
	return response["result"].toString().mid(2);
}
