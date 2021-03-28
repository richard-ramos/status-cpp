#pragma once

#include "balance-watcher.hpp"
#include "price-watcher.hpp"
#include "token-model.hpp"

#include "wallet.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QTimer>
#include <QVector>

namespace Wallet
{
const QString Generated = "generated";
const QString Seed = "seed";
const QString Key = "key";
const QString Watch = "watch";

struct Asset
{
	QString symbol;
	QString balance;
	QString name;

	Q_PROPERTY(QString symbol MEMBER symbol)
	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QString balance MEMBER balance)
	Q_GADGET
};

class WalletModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum WalletRoles
	{
		Name = Qt::UserRole + 1,
		Address = Qt::UserRole + 2,
		Color = Qt::UserRole + 3,
		WalletType = Qt::UserRole + 4,
		Path = Qt::UserRole + 5,
		Balances = Qt::UserRole + 6
	};

	explicit WalletModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void populateModel(QVector<Wallet*> wallets);

	Q_INVOKABLE void push(Wallet* wallet);
	//Q_INVOKABLE void remove(QString address);

	Q_INVOKABLE void generateNewAccount(QString password, QString name, QString color);
	Q_INVOKABLE void addAccountFromSeed(QString seed, QString password, QString name, QString color);
	Q_INVOKABLE void addAccountFromPrivateKey(QString privateKey, QString password, QString name, QString color);
	Q_INVOKABLE void addWatchOnlyAccount(QString address, QString name, QString color);

	Q_INVOKABLE QString getDefaultAccount();
	Q_INVOKABLE bool validatePassword(QString password);
	Q_INVOKABLE void fetchPrices();
	Q_INVOKABLE void fetchBalances();
	Q_INVOKABLE void setupWatchers();
	Q_INVOKABLE void updateBalance(QString address, QMap<QString, QString> balances);
	Q_INVOKABLE void updatePrices(QString currency, QMap<QString, double> prices);
	Q_INVOKABLE QVariant balances(int i) const;

	QML_READONLY_PROPERTY(QString, id)
	QML_READONLY_PROPERTY(bool, balancesLoaded);
	QML_READONLY_PROPERTY(bool, pricesLoaded);
	QML_WRITABLE_PROPERTY(TokenModel*, tokens);

	Q_PROPERTY(QMap<QString, QVariant> prices READ getPrices NOTIFY pricesChanged)
	Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

	QMap<QString, QVariant> getPrices();

signals:
	void walletsLoaded(QVector<Wallet*> wallets);
	void walletLoaded(Wallet* wallet);
	void invalidPassword();
	void accountCreated(bool success, QString message = "");
	void pricesChanged();
	void rowCountChanged();

private:
	void loadWallets();
	void insert(Wallet* wallet);
	QVector<Asset> getAssetList(QMap<QString, QString> balances);

	QVector<QSharedPointer<Wallet>> m_wallets;

	QTimer* priceWatcherTimer;
	QTimer* balanceWatcherTimer;

	PriceWatcher* priceWatcher;
	BalanceWatcher* balanceWatcher;

	QMap<QString, QVector<Asset>> m_balances;
	QMap<QString, QVariant> m_prices;
};

} // namespace Wallet