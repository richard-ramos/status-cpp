#pragma once

#include <QDebug>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QQmlHelpers>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

namespace Wallet
{
class Wallet : public QObject
{
	Q_OBJECT

public:
	explicit Wallet(QObject* parent = nullptr);
	explicit Wallet(const QJsonObject data, QObject* parent = nullptr);
	explicit Wallet(QString name, QString address, QString walletType, QString iconColor, QObject* parent = nullptr);

	QML_READONLY_PROPERTY(QString, name)
	QML_READONLY_PROPERTY(QString, address)
	QML_READONLY_PROPERTY(QString, walletType)
	QML_READONLY_PROPERTY(QString, iconColor)
};
} // namespace Wallet
