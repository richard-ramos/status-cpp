#include "wallet.hpp"

namespace Wallet
{
Wallet::Wallet(QObject* parent)
	: QObject(parent)
{ }

Wallet::Wallet(QJsonObject data, QObject* parent)
	: QObject(parent)
	, m_name(data["name"].toString())
	, m_address(data["address"].toString())
	, m_walletType(data["type"].toString())
	, m_iconColor(data["color"].toString())
{ }

Wallet::Wallet(QString name, QString address, QString walletType, QString iconColor, QObject* parent)
	: QObject(parent)
	, m_name(name)
	, m_address(address)
	, m_walletType(walletType)
	, m_iconColor(iconColor)
{ }

} // namespace Wallet