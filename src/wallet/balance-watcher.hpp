#pragma once

#include "token-model.hpp"
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQmlHelpers>
#include <QReadWriteLock>
#include <QString>
#include <QThread>
#include <QVector>

struct AccountBalance
{
	QString address;
	QMap<QString, QString> balances;
};

class BalanceWatcher : public QObject
{
	Q_OBJECT

public:
	BalanceWatcher(TokenModel* tokens, QObject* parent = nullptr);
	~BalanceWatcher();

	Q_INVOKABLE void fetch();

private:
	TokenModel* m_tokens;

	QVector<AccountBalance> queryBalances();
	QString getETHBalance(QString address);
	QString getTokenBalance(QString address, QString tokenAddress);

signals:
	void balanceFetched(QString address, QMap<QString, QString> balanceMap);
};