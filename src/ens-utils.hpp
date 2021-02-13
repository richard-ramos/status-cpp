#pragma once

#include <QObject>
#include <QString>
#include <QJSValue>

namespace Ens
{
const QString StatusDomain(".stateofus.eth");
const QString RegistryAddress("0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e");
const QString PubkeySignature("0xc8690233"); // pubkey(bytes32 node))
const QString ResolverSignature("0x0178b8bf");

class Utils : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString formatUsername(QString username);
	Q_INVOKABLE QString pubKey(QString username);
	Q_INVOKABLE void pubKey(QString username, const QJSValue& callback);

private:
	QString resolver(QString usernameHash);
	QString namehash(QString ensName);
	
};

} // namespace Ens
