#pragma once

#include "wallet.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>

namespace Wallet
{
const QString Generated = "generated";
const QString Seed = "seed";
const QString Key = "key";
const QString Watch = "watch";

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
		Path = Qt::UserRole + 5
	};

	explicit WalletModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(Wallet* wallet);
	//Q_INVOKABLE void remove(QString address);

	Q_INVOKABLE void generateNewAccount(QString password, QString name, QString color);
	Q_INVOKABLE void addAccountFromSeed(QString seed, QString password, QString name, QString color);
	Q_INVOKABLE void addAccountFromPrivateKey(QString privateKey, QString password, QString name, QString color);
	Q_INVOKABLE void addWatchOnlyAccount(QString address, QString name, QString color);
	
	Q_INVOKABLE QString getDefaultAccount();
	Q_INVOKABLE bool validatePassword(QString password);

signals:
	void walletLoaded(Wallet* wallet);
	void invalidPassword();
	void accountCreated(bool success, QString message = "");


private:
	void loadWallets();
	void insert(Wallet* wallet);

	QVector<QSharedPointer<Wallet>> m_wallets;
};

} // namespace Wallet