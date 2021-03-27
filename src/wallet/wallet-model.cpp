#include "wallet-model.hpp"
#include "constants.hpp"
#include "libstatus.h"
#include "price-watcher.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "token-model.hpp"
#include "utils.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QSharedPointer>
#include <QTimer>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

namespace Wallet
{

WalletModel::WalletModel(QObject* parent)
	: QAbstractListModel(parent)
{
	loadWallets();

	QObject::connect(this, &WalletModel::walletsLoaded, this, &WalletModel::populateModel);
	QObject::connect(this, &WalletModel::walletLoaded, this, &WalletModel::push);
	QObject::connect(this, &WalletModel::tokensChanged, this, &WalletModel::setupWatchers);

	connect(this, &QAbstractListModel::rowsInserted, this, &WalletModel::rowCountChanged);
	connect(this, &QAbstractListModel::rowsRemoved, this, &WalletModel::rowCountChanged);
	connect(this, &QAbstractListModel::dataChanged, this, &WalletModel::rowCountChanged);
	connect(this, &QAbstractListModel::modelReset, this, &WalletModel::rowCountChanged);
}

void WalletModel::setupWatchers()
{
	priceWatcher = new PriceWatcher(this);
	priceWatcherTimer = new QTimer(this);
	balanceWatcher = new BalanceWatcher(m_tokens, this);
	balanceWatcherTimer = new QTimer(this);

	// TODO: fetch when sending
	QObject::connect(this, &WalletModel::accountCreated, this, &WalletModel::fetchPrices);
	QObject::connect(this, &WalletModel::accountCreated, this, &WalletModel::fetchBalances);
	QObject::connect(Status::instance(), &Status::logout, priceWatcherTimer, &QTimer::stop);
	QObject::connect(Status::instance(), &Status::logout, balanceWatcherTimer, &QTimer::stop);
	QObject::connect(Settings::instance(), &Settings::currencyChanged, this, &WalletModel::fetchPrices);
	QObject::connect(Settings::instance(), &Settings::currencyChanged, this, &WalletModel::fetchBalances);
	QObject::connect(Settings::instance(), &Settings::visibleTokensChanged, this, &WalletModel::fetchPrices);
	QObject::connect(Settings::instance(), &Settings::visibleTokensChanged, this, &WalletModel::fetchBalances);

	QObject::connect(balanceWatcher, &BalanceWatcher::balanceFetched, this, &WalletModel::updateBalance);
	QObject::connect(priceWatcher, &PriceWatcher::priceUpdated, this, &WalletModel::updatePrices);
	QObject::connect(balanceWatcher, &BalanceWatcher::balanceFetched, this, &WalletModel::rowCountChanged);
	QObject::connect(priceWatcher, &PriceWatcher::priceUpdated, this, &WalletModel::rowCountChanged);
}

void WalletModel::fetchPrices()
{
	// Fire immediately
	priceWatcher->fetch();
	QObject::connect(priceWatcherTimer, &QTimer::timeout, priceWatcher, &PriceWatcher::fetch);
	priceWatcherTimer->start(300000); //
}

void WalletModel::fetchBalances()
{
	// Fire immediately
	balanceWatcher->fetch();
	QObject::connect(balanceWatcherTimer, &QTimer::timeout, balanceWatcher, &BalanceWatcher::fetch);
	priceWatcherTimer->start(300000);
}

QVector<Asset> WalletModel::getAssetList(QMap<QString, QString> balances)
{
	QVector<Asset> b;
	for(auto symbol : balances.keys())
	{
		auto tokenData = m_tokens->token(symbol);
		QString tokenName = tokenData.has_value() ? tokenData.value().name : "";
		Asset a{.symbol = symbol, .balance = balances[symbol], .name = tokenName};
		if(symbol == "ETH")
		{
			b.insert(0, a);
		}
		else if(symbol == "SNT" || symbol == "STT")
		{
			if(b.size() > 1)
			{
				b.insert(b[0].symbol == "ETH" ? 1 : 0, a);
			}
			else
			{
				b << a;
			}
		}
		else
		{
			b << a;
		}
	}
	return b;
}

void WalletModel::updateBalance(QString address, QMap<QString, QString> balances)
{
	m_balances[address] = getAssetList(balances);

	for(int i = 0; i < m_wallets.size(); i++)
	{
		if(m_wallets[i]->get_address() == address)
		{
			QModelIndex idx = createIndex(i, 0);
			dataChanged(idx, idx);
		}
	}

	update_balancesLoaded(true);
}

void WalletModel::updatePrices(QString currency, QMap<QString, double> prices)
{
	QMap<QString, QVariant> newPrices;
	if(QString::compare(currency, Settings::instance()->currency(), Qt::CaseInsensitive) == 0)
	{
		for(auto symbol : prices.keys())
		{
			newPrices[symbol] = QVariant(prices[symbol]);
		}
	}
	m_prices = newPrices;
	emit pricesChanged();
	update_pricesLoaded(true);
}

QMap<QString, QVariant> WalletModel::getPrices()
{
	return m_prices;
}

QHash<int, QByteArray> WalletModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Name] = "name";
	roles[Address] = "address";
	roles[Color] = "iconColor";
	roles[WalletType] = "walletType";
	roles[Path] = "path";
	roles[Balances] = "balances";
	return roles;
}

int WalletModel::rowCount(const QModelIndex& parent) const
{
	return m_wallets.size();
}

QVariant WalletModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	QSharedPointer<Wallet> t = m_wallets[index.row()];

	switch(role)
	{
	case Name: return QVariant(t->get_name());
	case Path: return QVariant(t->get_path());
	case Address: return QVariant(t->get_address());
	case Color: return QVariant(t->get_iconColor());
	case WalletType: return QVariant(t->get_walletType());
	case Balances: return QVariant::fromValue(m_balances[t->get_address()]);
	}

	return QVariant();
}

QVariant WalletModel::balances(int i) const
{
	QSharedPointer<Wallet> t = m_wallets[i];
	return QVariant::fromValue(m_balances[t->get_address()]);
}

void WalletModel::loadWallets()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("accounts_getAccounts", QJsonArray{}.toVariantList()).toJsonObject();
		QVector<Wallet*> wallets;
		foreach(QJsonValue accountJson, response["result"].toArray())
		{
			const QJsonObject accountObj = accountJson.toObject();

			if(accountObj["chat"].toBool() == true) continue; // Might need a better condition
			Wallet* wallet = new Wallet(accountObj);
			wallet->moveToThread(QApplication::instance()->thread());
			wallets << wallet;
		}
		emit walletsLoaded(wallets);
	});
}

void WalletModel::populateModel(QVector<Wallet*> wallets)
{
	foreach(Wallet* w, wallets)
	{
		push(w);
	}

	fetchPrices();
	fetchBalances();
}

void WalletModel::push(Wallet* wallet)
{
	QQmlApplicationEngine::setObjectOwnership(wallet, QQmlApplicationEngine::CppOwnership);
	insert(wallet);
}

void WalletModel::insert(Wallet* token)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_wallets << QSharedPointer<Wallet>(token);
	endInsertRows();
}

QJsonObject saveAccountsRPCCall(QString name, QString color, QString address, QString publicKey, QString accountType, QString path)
{
	return Status::instance()
		->callPrivateRPC(
			"accounts_saveAccounts",
			QJsonArray{QJsonArray{QJsonObject{
						   {"color", color}, {"name", name}, {"address", address}, {"public-key", publicKey}, {"type", accountType}, {"path", path}}}}
				.toVariantList())
		.toJsonObject();
}

void WalletModel::addWatchOnlyAccount(QString address, QString name, QString color)
{
	const QJsonObject response = saveAccountsRPCCall(name, color, address, "", Watch, Constants::PathDefaultWallet);

	if(Status::isError(response))
	{
		qCritical() << Status::errorMessage(response);
		emit accountCreated(false, Status::errorMessage(response));
		return;
	}

	emit walletLoaded(new Wallet(name, address, Watch, color, Constants::PathDefaultWallet));
	emit accountCreated(true);
}

QString WalletModel::getDefaultAccount()
{
	const auto response = Status::instance()->callPrivateRPC("eth_accounts", QJsonArray{}.toVariantList()).toJsonObject();
	// TODO: error handling
	return response["result"].toArray()[0].toString();
}

bool WalletModel::validatePassword(QString hashedPassword)
{
	const auto verifyResult = VerifyAccountPassword(
		Constants::applicationPath(Constants::Keystore).toUtf8().data(), getDefaultAccount().toUtf8().data(), hashedPassword.toUtf8().data());
	const QJsonObject verifyResultJson = QJsonDocument::fromJson(verifyResult).object();

	return verifyResultJson["error"].isUndefined() || verifyResultJson["error"].toString().isEmpty();
}

void WalletModel::addAccountFromSeed(QString seed, QString password, QString name, QString color)
{
	seed = seed.replace(',', ' ');

	QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	if(!validatePassword(hashedPassword))
	{
		emit invalidPassword();
		return;
	}

	const char* importResult =
		MultiAccountImportMnemonic(Utils::jsonToStr(QJsonObject{{"mnemonicPhrase", seed}, {"Bip39Passphrase", ""}}).toUtf8().data());

	QJsonObject importResultObj = QJsonDocument::fromJson(importResult).object();

	if(Status::isError(importResultObj))
	{
		qCritical() << Status::errorMessage(importResultObj);
		emit accountCreated(false, Status::errorMessage(importResultObj));
		return;
	}

	const char* deriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(QJsonObject{{"accountID", importResultObj["id"]}, {"paths", QJsonArray{Constants::PathDefaultWallet}}}).toUtf8().data());
	QJsonObject deriveResultObj = QJsonDocument::fromJson(deriveResult).object();

	if(Status::isError(deriveResultObj))
	{
		qCritical() << Status::errorMessage(deriveResultObj);
		emit accountCreated(false, Status::errorMessage(deriveResultObj));
		return;
	}

	const char* storeDerivedResult = MultiAccountStoreDerivedAccounts(
		Utils::jsonToStr(
			QJsonObject{{"accountID", importResultObj["id"]}, {"paths", QJsonArray{Constants::PathDefaultWallet}}, {"password", hashedPassword}})
			.toUtf8()
			.data());

	QJsonObject storeDerivedResultObj = QJsonDocument::fromJson(storeDerivedResult).object();

	if(Status::isError(storeDerivedResultObj))
	{
		qCritical() << Status::errorMessage(storeDerivedResultObj);
		emit accountCreated(false, Status::errorMessage(storeDerivedResultObj));
		return;
	}

	QString publicKey = deriveResultObj[Constants::PathDefaultWallet].toObject()["publicKey"].toString();
	QString address = deriveResultObj[Constants::PathDefaultWallet].toObject()["address"].toString();

	const QJsonObject saveAccountsResponseObj = saveAccountsRPCCall(name, color, address, publicKey, Seed, Constants::PathDefaultWallet);

	if(Status::isError(saveAccountsResponseObj))
	{
		qCritical() << Status::errorMessage(saveAccountsResponseObj);
		emit accountCreated(false, Status::errorMessage(saveAccountsResponseObj));
		return;
	}

	emit walletLoaded(new Wallet(name, address, Seed, color, Constants::PathDefaultWallet));
	emit accountCreated(true);
}

void WalletModel::addAccountFromPrivateKey(QString privateKey, QString password, QString name, QString color)
{
	QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	if(!validatePassword(hashedPassword))
	{
		emit invalidPassword();
		return;
	}

	const char* importResult = MultiAccountImportPrivateKey(Utils::jsonToStr(QJsonObject{{"privateKey", privateKey}}).toUtf8().data());
	QJsonObject importResultObj = QJsonDocument::fromJson(importResult).object();

	if(Status::isError(importResultObj))
	{
		qCritical() << Status::errorMessage(importResultObj);
		emit accountCreated(false, Status::errorMessage(importResultObj));
		return;
	}

	const char* storeResult =
		MultiAccountStoreAccount(Utils::jsonToStr(QJsonObject{{"accountID", importResultObj["id"]}, {"password", hashedPassword}}).toUtf8().data());
	QJsonObject storeResultObj = QJsonDocument::fromJson(storeResult).object();

	if(Status::isError(storeResultObj))
	{
		qCritical() << Status::errorMessage(storeResultObj);
		emit accountCreated(false, Status::errorMessage(storeResultObj));
		return;
	}

	QString publicKey = importResultObj["publicKey"].toString();
	QString address = importResultObj["address"].toString();

	const QJsonObject saveAccountsResultObj = saveAccountsRPCCall(name, color, address, publicKey, Key, Constants::PathDefaultWallet);

	if(Status::isError(saveAccountsResultObj))
	{
		qCritical() << Status::errorMessage(saveAccountsResultObj);
		emit accountCreated(false, Status::errorMessage(saveAccountsResultObj));
		return;
	}

	emit walletLoaded(new Wallet(name, address, Key, color, Constants::PathDefaultWallet));
	emit accountCreated(true);
}

void WalletModel::generateNewAccount(QString password, QString name, QString color)
{
	QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	if(!validatePassword(hashedPassword))
	{
		emit invalidPassword();
		return;
	}

	QString walletRootAddress = Settings::instance()->walletRootAddress();
	int walletIndex = Settings::instance()->latestDerivedPath() + 1;

	const char* loadAccountResult =
		MultiAccountLoadAccount(Utils::jsonToStr(QJsonObject{{"address", walletRootAddress}, {"password", hashedPassword}}).toUtf8().data());
	QJsonObject loadAccountResultObj = QJsonDocument::fromJson(loadAccountResult).object();

	if(Status::isError(loadAccountResultObj))
	{
		qCritical() << Status::errorMessage(loadAccountResultObj);
		emit accountCreated(false, Status::errorMessage(loadAccountResultObj));
		return;
	}

	QString mPath = "m/" + QString::number(walletIndex);

	const char* multiaccountDeriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(QJsonObject{{"accountID", loadAccountResultObj["id"]}, {"paths", QJsonArray{mPath}}}).toUtf8().data());
	QJsonObject multiaccountDeriveResultObj = QJsonDocument::fromJson(multiaccountDeriveResult).object();

	if(Status::isError(multiaccountDeriveResultObj))
	{
		qCritical() << Status::errorMessage(multiaccountDeriveResultObj);
		emit accountCreated(false, Status::errorMessage(multiaccountDeriveResultObj));
		return;
	}

	const char* multiaccountStoreDeriveResult = MultiAccountStoreDerivedAccounts(
		Utils::jsonToStr(QJsonObject{{"accountID", loadAccountResultObj["id"]}, {"paths", QJsonArray{mPath}}, {"password", hashedPassword}})
			.toUtf8()
			.data());
	QJsonObject multiaccountStoreDeriveResultObj = QJsonDocument::fromJson(multiaccountStoreDeriveResult).object();

	if(Status::isError(multiaccountStoreDeriveResultObj))
	{
		qCritical() << Status::errorMessage(multiaccountStoreDeriveResultObj);
		emit accountCreated(false, Status::errorMessage(multiaccountStoreDeriveResultObj));
		return;
	}

	QString publicKey = multiaccountDeriveResultObj[mPath].toObject()["publicKey"].toString();
	QString address = multiaccountDeriveResultObj[mPath].toObject()["address"].toString();
	QString path = "m/44'/60'/0'/0/" + QString::number(walletIndex);

	const QJsonObject saveAccountResponseObj = saveAccountsRPCCall(name, color, address, publicKey, Generated, path);
	if(Status::isError(saveAccountResponseObj))
	{
		qCritical() << Status::errorMessage(saveAccountResponseObj);
		emit accountCreated(false, Status::errorMessage(saveAccountResponseObj));
		return;
	}

	Settings::instance()->setLatestDerivedPath(walletIndex);

	emit walletLoaded(new Wallet(name, address, Generated, color, path));
	emit accountCreated(true);
}

} // namespace Wallet