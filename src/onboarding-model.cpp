#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QUuid>
#include <QtConcurrent>

#include <algorithm>
#include <array>

#include "constants.hpp"
#include "libstatus.h"
#include "onboarding-model.hpp"
#include "signing-phrases.hpp"
#include "utils.hpp"

OnboardingModel::OnboardingModel(QObject* parent)
	: QAbstractListModel(parent)
{ }

GeneratedAccount jsonObjectToAccount(const QJsonObject obj)
{
	GeneratedAccount acc = {};
	acc.id = obj["id"].toString();
	acc.address = obj["address"].toString();
	acc.keyUid = obj["keyUid"].toString();
	acc.mnemonic = obj["mnemonic"].toString(); // TODO: clear from memory?
	acc.publicKey = obj["publicKey"].toString();
	return acc;
}

void setDerivedKeys(GeneratedAccount& acc, const QJsonObject obj)
{
	acc.derivedKeys = {{Constants::PathWalletRoot,
						{obj[Constants::PathWalletRoot]["publicKey"].toString(),
						 obj[Constants::PathWalletRoot]["address"].toString()}},
					   {Constants::PathEip1581,
						{obj[Constants::PathEip1581]["publicKey"].toString(),
						 obj[Constants::PathEip1581]["address"].toString()}},
					   {Constants::PathWhisper,
						{obj[Constants::PathWhisper]["publicKey"].toString(),
						 obj[Constants::PathWhisper]["address"].toString()}},
					   {Constants::PathDefaultWallet,
						{obj[Constants::PathDefaultWallet]["publicKey"].toString(),
						 obj[Constants::PathDefaultWallet]["address"].toString()}}};
}

QVector<GeneratedAccount> multiAccountGenerateAndDeriveAddresses()
{
	QJsonObject obj{{"n", 5},
					{"mnemonicPhraseLength", 12},
					{"bip32Passphrase", ""},
					{"paths",
					 QJsonArray{Constants::PathWalletRoot,
								Constants::PathEip1581,
								Constants::PathWhisper,
								Constants::PathDefaultWallet}}

	};
	const char* result = MultiAccountGenerateAndDeriveAddresses(
		Utils::jsonToStr(obj).toUtf8().data()); // TODO: clear from memory
	QJsonArray multiAccounts = QJsonDocument::fromJson(result).array();

	QVector<GeneratedAccount> vector;
	foreach(const QJsonValue& value, multiAccounts)
	{
		GeneratedAccount acc = jsonObjectToAccount(value.toObject());
		setDerivedKeys(acc, value.toObject()["derived"].toObject());
		vector.append(acc);
	}
	return vector;
}

void OnboardingModel::populate()
{
	beginResetModel();
	mData.clear();
	mData << multiAccountGenerateAndDeriveAddresses();
	endResetModel();
}

// TODO: destructor. Clear

QHash<int, QByteArray> OnboardingModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "id";
	roles[PublicKey] = "publicKey";
	roles[Name] = "name";
	roles[Image] = "image";
	return roles;
}

int OnboardingModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return mData.size();
}

QVariant OnboardingModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	switch(role)
	{
	case Id:
		return QVariant(mData[index.row()].keyUid);
	case PublicKey:
		return QVariant(mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey);
	case Image:
		return QVariant(Utils::generateIdenticon(
			mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey));
	case Name:
		return QVariant(Utils::generateAlias(
			mData[index.row()].derivedKeys.at(Constants::PathWhisper).publicKey));
	}

	return QVariant();
}

QString OnboardingModel::getAccountId(int index)
{
	return mData[index].keyUid;
}

QVariantMap OnboardingModel::get(int row) const
{
	QHash<int, QByteArray> names = roleNames();
	QHashIterator<int, QByteArray> i(names);
	QVariantMap res;
	QModelIndex idx = index(row, 0);
	while(i.hasNext())
	{
		i.next();
		QVariant data = idx.data(i.key());
		res[i.value()] = data;
	}
	return res;
}

void storeDerivedAccount(QString& accountId, QString& hashedPassword)
{
	QJsonObject obj{{"accountID", accountId},
					{"paths",
					 QJsonArray{Constants::PathWalletRoot,
								Constants::PathEip1581,
								Constants::PathWhisper,
								Constants::PathDefaultWallet}},
					{"password", hashedPassword}};

	MultiAccountStoreDerivedAccounts(Utils::jsonToStr(obj).toUtf8().data());

	// TODO: clear obj
}

QJsonObject getAccountData(GeneratedAccount* account)
{
	return QJsonObject{
		{"name", Utils::generateAlias(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"address", account->address},
		{"photo-path",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"identicon",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"key-uid", account->keyUid},
		{"keycard-pairing", QJsonValue()}};
}

QString generateSigningPhrase(int count)
{
	QStringList words;
	for(int i = 0; i < count; i++)
	{
		words.append(
			phrases[QRandomGenerator::global()->bounded(static_cast<int>(phrases.size()))]);
	}
	return words.join(" ");
}

QJsonObject getAccountSettings(GeneratedAccount* account, QString installationId)
{
	QFile defaultNetworks(":/resources/default-networks.json");
	defaultNetworks.open(QIODevice::ReadOnly);

	QString defaultNetworksContent = defaultNetworks.readAll();
	defaultNetworksContent = defaultNetworksContent.replace("%INFURA_KEY%", INFURA_KEY);

	QJsonArray defaultNetworksJson =
		QJsonDocument::fromJson(defaultNetworksContent.toUtf8()).array();

	return QJsonObject{
		{"key-uid", account->keyUid},
		{"mnemonic", account->mnemonic},
		{"public-key", account->derivedKeys[Constants::PathWhisper].publicKey},
		{"name", Utils::generateAlias(account->publicKey)},
		{"address", account->address},
		{"eip1581-address", account->derivedKeys[Constants::PathEip1581].address},
		{"dapps-address", account->derivedKeys[Constants::PathDefaultWallet].address},
		{"wallet-root-address", account->derivedKeys[Constants::PathDefaultWallet].address},
		{"preview-privacy?", true},
		{"signing-phrase", generateSigningPhrase(3)},
		{"log-level", "INFO"},
		{"latest-derived-path", 0},
		{"networks/networks", defaultNetworksJson},
		{"currency", "usd"},
		{"identicon",
		 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
		{"waku-enabled", true},
		{"wallet/visible-tokens", {{"mainnet", QJsonArray{"SNT"}}}},
		{"appearance", 0},
		{"networks/current-network", Constants::DefaultNetworkName},
		{"installation-id", installationId}};

	// TODO: clear mnemonic?
}

QJsonObject getNodeConfig(QString installationId)
{
	QFile nodeConfig(":/resources/node-config.json");
	nodeConfig.open(QIODevice::ReadOnly);

	QString nodeConfigContent = nodeConfig.readAll();

	nodeConfigContent = nodeConfigContent.replace("%INSTALLATIONID%", installationId);
	nodeConfigContent = nodeConfigContent.replace("%INFURA_KEY%", INFURA_KEY);

	QJsonObject nodeConfigJson = QJsonDocument::fromJson(nodeConfigContent.toUtf8()).object();

	QFile fleets(":/resources/fleets.json");
	fleets.open(QIODevice::ReadOnly);
	QJsonObject fleetJson = QJsonDocument::fromJson(fleets.readAll())
								.object()["fleets"]
								.toObject()["eth.prod"]
								.toObject();

	QJsonObject clusterConfig = nodeConfigJson["ClusterConfig"].toObject();

	auto boot = fleetJson["boot"].toObject();
	QJsonArray bootNodes;
	for(auto it = boot.begin(); it != boot.end(); ++it)
		bootNodes << *it;
	clusterConfig["BootNodes"] = bootNodes;

	auto rendezvous = fleetJson["rendezvous"].toObject();
	QJsonArray rendezvousNodes;
	for(auto it = rendezvous.begin(); it != rendezvous.end(); ++it)
		rendezvousNodes << *it;
	clusterConfig["RendezvousNodes"] = rendezvousNodes;

	auto whisper = fleetJson["whisper"].toObject();
	QJsonArray staticNodes;
	for(auto it = whisper.begin(); it != whisper.end(); ++it)
		staticNodes << *it;
	clusterConfig["StaticNodes"] = staticNodes;

	auto mail = fleetJson["mail"].toObject();
	QJsonArray trustedMailserverNodes;
	for(auto it = mail.begin(); it != mail.end(); ++it)
		trustedMailserverNodes << *it;
	clusterConfig["TrustedMailServers"] = trustedMailserverNodes;

	nodeConfigJson["ClusterConfig"] = clusterConfig;

	return nodeConfigJson;
}

QJsonArray getSubAccountData(GeneratedAccount* account)
{
	return QJsonArray{
		QJsonObject{{"public-key", account->derivedKeys[Constants::PathDefaultWallet].publicKey},
					{"address", account->derivedKeys[Constants::PathDefaultWallet].address},
					{"color", "#4360df"},
					{"wallet", true},
					{"path", Constants::PathDefaultWallet},
					{"name", "Status account"}},
		QJsonObject{
			{"public-key", account->derivedKeys[Constants::PathWhisper].publicKey},
			{"address", account->derivedKeys[Constants::PathWhisper].address},
			{"path", Constants::PathWhisper},
			{"name", Utils::generateAlias(account->derivedKeys[Constants::PathWhisper].publicKey)},
			{"identicon",
			 Utils::generateIdenticon(account->derivedKeys[Constants::PathWhisper].publicKey)},
			{"chat", true}}};
}

bool saveAccountAndLogin(GeneratedAccount* genAccount, QString password)
{
	QString hashedPassword = QString::fromUtf8(
		QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));

	storeDerivedAccount(genAccount->id, hashedPassword);

	QString installationId(QUuid::createUuid().toString(QUuid::WithoutBraces));
	QJsonObject accountData(getAccountData(genAccount));
	QJsonObject settings(getAccountSettings(genAccount, installationId));
	QJsonObject nodeConfig(getNodeConfig(installationId));
	QJsonArray subAccountData(getSubAccountData(genAccount));

	qDebug() << SaveAccountAndLogin(Utils::jsonToStr(accountData).toUtf8().data(),
									hashedPassword.toUtf8().data(),
									Utils::jsonToStr(settings).toUtf8().data(),
									Utils::jsonToStr(nodeConfig).toUtf8().data(),
									Utils::jsonToStr(subAccountData).toUtf8().data());

	return true;
	// TODO: clear hashedPassword, genAccount
}

QString OnboardingModel::validateMnemonic(QString mnemonic)
{
	// TODO: clear memory
	const char* result(ValidateMnemonic(mnemonic.toUtf8().data()));
	const QJsonObject obj = QJsonDocument::fromJson(result).object();
	return obj["error"].toString();
}

QString OnboardingModel::importMnemonic(QString mnemonic)
{
	// TODO: clear memory
	QJsonObject obj1{{"mnemonicPhrase", mnemonic}, {"Bip39Passphrase", ""}};
	const char* importResult = MultiAccountImportMnemonic(
		Utils::jsonToStr(obj1).toUtf8().data()); // TODO: clear from memory
	GeneratedAccount acc = jsonObjectToAccount(QJsonDocument::fromJson(importResult).object());

	QJsonObject obj2{{"accountID", acc.id},
					 {"paths",
					  QJsonArray{Constants::PathWalletRoot,
								 Constants::PathEip1581,
								 Constants::PathWhisper,
								 Constants::PathDefaultWallet}}};
	const char* deriveResult = MultiAccountDeriveAddresses(
		Utils::jsonToStr(obj2).toUtf8().data()); // TODO: clear from memory
	setDerivedKeys(acc, QJsonDocument::fromJson(deriveResult).object());

	beginResetModel();
	mData.clear();
	mData << acc;
	endResetModel();

	return acc.id;
}

void OnboardingModel::setup(QString accountId, QString password)
{
	auto genAccount =
		std::find_if(mData.begin(), mData.end(), [accountId](const GeneratedAccount& m) -> bool {
			return m.keyUid == accountId;
		});
	if(genAccount != mData.end())
	{
		QtConcurrent::run([=] {
			bool result(saveAccountAndLogin(genAccount, password));

			// TODO: clear mnemonic from memory in list

			emit accountSaved(result);
		});
	}

	// TODO: error handling
	// TODO: clear password
}