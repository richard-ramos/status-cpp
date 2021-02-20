#include "ens-utils.hpp"
#include "constants.hpp"
#include "status.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringBuilder>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

QString Ens::Utils::formatUsername(QString username)
{
	if(username.endsWith(".eth"))
	{
		return username;
	}
	else
	{
		return username + Constants::StatusDomain;
	}
}

static inline QStringList stdReverseList(const QStringList& list)
{
	QStringList reversedList = list;
	std::reverse(reversedList.begin(), reversedList.end());
	return reversedList;
}

QString Ens::Utils::namehash(QString ensName)
{
	QString name(ensName.toLower());
	QByteArray node(32, 0);

	foreach(const QString& part, stdReverseList(name.split('.')))
	{
		QByteArray elem = QCryptographicHash::hash(part.toUtf8(), QCryptographicHash::Keccak_256);
		QByteArray concatArray;
		concatArray.append(node);
		concatArray.append(elem);
		node = QCryptographicHash::hash(concatArray, QCryptographicHash::Keccak_256);
	}
	return QString(node.toHex());
}

QString Ens::Utils::resolver(QString usernameHash)
{
	QJsonObject payload{
		{"to", RegistryAddress},
		{"from", Constants::ZeroAddress},
		{"data", ResolverSignature + usernameHash},
	};

	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();
	// TODO: error handling

	return "0x" + response["result"].toString().right(40);
}

QString Ens::Utils::pubKey(QString username)
{
	QString usernameHash(namehash(formatUsername(username)));
	QString ensResolver(resolver(usernameHash));

	if(ensResolver == Constants::ZeroAddress)
		return "";

	QJsonObject payload{
		{"to", ensResolver},
		{"from", Constants::ZeroAddress},
		{"data", PubkeySignature + usernameHash},
	};

	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();

	QString publicKey(response["result"].toString());

	if(publicKey == "0x" or
	   publicKey ==
		   "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000")
		return "";

	return "0x04" + publicKey.right(128);
}

QString Ens::Utils::address(QString username)
{
	QString usernameHash(namehash(formatUsername(username)));
	QString ensResolver(resolver(usernameHash));

	if(ensResolver == Constants::ZeroAddress)
		return "";

	QJsonObject payload{
		{"to", ensResolver},
		{"from", Constants::ZeroAddress},
		{"data", AddressSignature + usernameHash},
	};

	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();

	QString address(response["result"].toString());

	if(address == "0x" or address == "0x0000000000000000000000000000000000000000000000000000000000000000")
		return "";

	return "0x" + address.right(40);
}

void Ens::Utils::pubKey(QString username, const QJSValue& callback)
{
	auto* watcher = new QFutureWatcher<QString>(this);
	QObject::connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, callback]() {
		QString result = watcher->result();
		QJSValue cbCopy(callback); // needed as callback is captured as const
		QJSEngine* engine = qjsEngine(this);
		cbCopy.call(QJSValueList{engine->toScriptValue(result)});
		watcher->deleteLater();
	});
	watcher->setFuture(QtConcurrent::run(this, &Ens::Utils::pubKey, username));
}

void Ens::Utils::address(QString username, const QJSValue& callback)
{
	auto* watcher = new QFutureWatcher<QString>(this);
	QObject::connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, callback]() {
		QString result = watcher->result();
		QJSValue cbCopy(callback); // needed as callback is captured as const
		QJSEngine* engine = qjsEngine(this);
		cbCopy.call(QJSValueList{engine->toScriptValue(result)});
		watcher->deleteLater();
	});
	watcher->setFuture(QtConcurrent::run(this, &Ens::Utils::address, username));
}

QString Ens::Utils::owner(QString username)
{
	QString usernameHash(namehash(formatUsername(username)));

	QJsonObject payload{
		{"to", RegistryAddress},
		{"from", Constants::ZeroAddress},
		{"data", OwnerSignature + usernameHash},
	};

	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();
	QString address(response["result"].toString());

	if(address == "0x" or address == "0x0000000000000000000000000000000000000000000000000000000000000000")
		return "";

	return "0x" + address.right(40);
}
