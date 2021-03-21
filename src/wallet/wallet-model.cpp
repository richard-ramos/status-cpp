#include "wallet-model.hpp"
#include "constants.hpp"
#include "libstatus.h"
#include "settings.hpp"
#include "status.hpp"
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
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

namespace Wallet
{

WalletModel::WalletModel(QObject* parent)
	: QAbstractListModel(parent)
{
	QObject::connect(this, &WalletModel::walletLoaded, this, &WalletModel::push);
	loadWallets();
}

QHash<int, QByteArray> WalletModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Name] = "name";
	roles[Address] = "address";
	roles[Color] = "iconColor";
	roles[WalletType] = "walletType";
	return roles;
}

int WalletModel::rowCount(const QModelIndex& parent = QModelIndex()) const
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
	case Address: return QVariant(t->get_address());
	case Color: return QVariant(t->get_iconColor());
	case WalletType: return QVariant(t->get_walletType());
	}

	return QVariant();
}

void WalletModel::loadWallets()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("accounts_getAccounts", QJsonArray{}.toVariantList()).toJsonObject();

		foreach(QJsonValue accountJson, response["result"].toArray())
		{
			const QJsonObject accountObj = accountJson.toObject();

			if(accountObj["chat"].toBool() == true) continue; // Might need a better condition
			Wallet* wallet = new Wallet(accountObj);
			wallet->moveToThread(QApplication::instance()->thread());

			emit walletLoaded(wallet);
		}
	});
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

	if(!response["error"].isUndefined())
	{
		// TODO: throw exception
	}

	Wallet* wallet = new Wallet(name, address, Watch, color);
	emit walletLoaded(wallet);
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
		qCritical() << "INVALID PASSWORD";
		return;
	}

	// TODO: error handling

	const char* importResult =
		MultiAccountImportMnemonic(Utils::jsonToStr(QJsonObject{{"mnemonicPhrase", seed}, {"Bip39Passphrase", ""}}).toUtf8().data());

	QJsonObject importResultObj = QJsonDocument::fromJson(importResult).object();

	const char* deriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(QJsonObject{{"accountID", importResultObj["id"]}, {"paths", QJsonArray{Constants::PathDefaultWallet}}}).toUtf8().data());
	QJsonObject deriveResultObj = QJsonDocument::fromJson(deriveResult).object();

	MultiAccountStoreDerivedAccounts(
		Utils::jsonToStr(
			QJsonObject{{"accountID", importResultObj["id"]}, {"paths", QJsonArray{Constants::PathDefaultWallet}}, {"password", hashedPassword}})
			.toUtf8()
			.data());

	QString publicKey = deriveResultObj[Constants::PathDefaultWallet].toObject()["publicKey"].toString();
	QString address = deriveResultObj[Constants::PathDefaultWallet].toObject()["address"].toString();

	const QJsonObject response = saveAccountsRPCCall(name, color, address, publicKey, Key, Constants::PathDefaultWallet);

	if(!response["error"].isUndefined())
	{
		// TODO: throw exception
	}

	Wallet* wallet = new Wallet(name, address, Key, color);
	emit walletLoaded(wallet);
}

void WalletModel::addAccountFromPrivateKey(QString privateKey, QString password, QString name, QString color)
{
	QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	if(!validatePassword(hashedPassword))
	{
		qCritical() << "INVALID PASSWORD";
		return;
	}

	// TODO: error handling

	const char* importResult = MultiAccountImportPrivateKey(Utils::jsonToStr(QJsonObject{{"privateKey", privateKey}}).toUtf8().data());
	QJsonObject importResultObj = QJsonDocument::fromJson(importResult).object();

	const char* storeResult =
		MultiAccountStoreAccount(Utils::jsonToStr(QJsonObject{{"accountID", importResultObj["id"]}, {"password", hashedPassword}}).toUtf8().data());
	QJsonObject storeResultObj = QJsonDocument::fromJson(storeResult).object();

	QString publicKey = importResultObj["publicKey"].toString();
	QString address = importResultObj["address"].toString();

	const QJsonObject response = saveAccountsRPCCall(name, color, address, publicKey, Key, Constants::PathDefaultWallet);

	if(!response["error"].isUndefined())
	{
		// TODO: throw exception
	}

	Wallet* wallet = new Wallet(name, address, Key, color);
	emit walletLoaded(wallet);
}

void WalletModel::generateNewAccount(QString password, QString name, QString color)
{
	QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	if(!validatePassword(hashedPassword))
	{
		qCritical() << "INVALID PASSWORD";
		return;
	}

	QString walletRootAddress = Settings::instance()->walletRootAddress();
	int walletIndex = Settings::instance()->latestDerivedPath() + 1;

	const char* loadAccountResult =
		MultiAccountLoadAccount(Utils::jsonToStr(QJsonObject{{"address", walletRootAddress}, {"password", hashedPassword}}).toUtf8().data());
	QJsonObject loadAccountResultObj = QJsonDocument::fromJson(loadAccountResult).object();

	QString mPath = "m/" + QString::number(walletIndex);

	const char* multiaccountDeriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(QJsonObject{{"accountID", loadAccountResultObj["id"]}, {"paths", QJsonArray{mPath}}}).toUtf8().data());

	QJsonObject multiaccountDeriveResultObj = QJsonDocument::fromJson(multiaccountDeriveResult).object();

	// generate account
	const char* multiaccountStoreDeriveResult = MultiAccountStoreDerivedAccounts(
		Utils::jsonToStr(QJsonObject{{"accountID", loadAccountResultObj["id"]}, {"paths", QJsonArray{mPath}}, {"password", hashedPassword}})
			.toUtf8()
			.data());

	QString publicKey = multiaccountDeriveResultObj[mPath].toObject()["publicKey"].toString();
	QString address = multiaccountDeriveResultObj[mPath].toObject()["address"].toString();

	const QJsonObject response = saveAccountsRPCCall(name, color, address, publicKey, Generated, "m/44'/60'/0'/0/" + walletIndex);

	if(!response["error"].isUndefined())
	{
		// TODO: throw exception
	}

	Settings::instance()->setLatestDerivedPath(walletIndex);

	Wallet* wallet = new Wallet(name, address, Generated, color);
	emit walletLoaded(wallet);
}

} // namespace Wallet