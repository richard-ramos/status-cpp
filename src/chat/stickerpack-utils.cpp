#include "stickerpack-utils.hpp"
#include "constants.hpp"
#include "status.hpp"
#include "stickerpack.hpp"
#include "uint256_t.h"
#include "utils.hpp"
#include <QCryptographicHash>
#include <QJsonObject>
#include <QString>
#include <QVector>

QString methodSignature(QString methodName)
{
	return "0x" + QCryptographicHash::hash(QString(methodName).toUtf8(), QCryptographicHash::Keccak_256).toHex().left(8);
}

int StickerPackUtils::getPackCount()
{
	QString packCount = methodSignature("packCount()");
	QJsonObject payload{
		{"to", Constants::StickersAddress},
		{"from", Constants::ZeroAddress},
		{"data", packCount},
	};
	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();
	// TODO: error handling

	bool ok = false;
	int result = response["result"].toString().right(response["result"].toString().size() - 2).toInt(&ok, 16);
	return ok ? result : 0;
}

StickerPack* StickerPackUtils::getPackData(int packId)
{
	QString getPackData = methodSignature("getPackData(uint256)");
	QJsonObject payload{
		{"to", Constants::StickersAddress},
		{"from", Constants::ZeroAddress},
		{"data", getPackData + QString::fromStdString(uint256_t(packId).str(16, 64))},
	};
	const auto response = Status::instance()->callPrivateRPC("eth_call", QJsonArray{payload, "latest"}.toVariantList()).toJsonObject();
	// TODO: error handling

	// TODO: extract to decode lib
	bool ok = false;
	QString data = response["result"].toString().right(response["result"].toString().size() - 2);

	if(data.isEmpty()) return nullptr; // TODO: throw exception or std::optional?

	QStringList category;
	int category_idx = data.mid(0, 64).toInt(&ok, 16);
	int category_cnt = data.mid(category_idx * 2, 64).toInt(&ok, 16);
	int curr_idx = (category_idx * 2 + 64);
	for(int i = 0; i < category_cnt; i++)
	{
		curr_idx += i * 64;
		category << "0x" + data.mid(curr_idx, 64).right(8);
	}

	QString owner = "0x" + data.mid(64, 64).right(40);
	bool mintable = data.mid(128, 64).right(1) == "1";
	QString timestamp = QString::number(data.mid(192, 64).toLongLong(&ok, 16));
	QString price = QString::fromStdString(uint256_t(data.mid(256, 64).toStdString()).str());

	int contentHash_idx = data.mid(320, 64).toInt(&ok, 16);
	int contentHash_length = data.mid(contentHash_idx * 2, 64).toInt(&ok, 16);

	qCritical() << data;

	QString contentHash = Utils::decodeHash(data.mid(contentHash_idx * 2 + 64, contentHash_length * 2));

	return new StickerPack(packId, category, owner, mintable, timestamp, price, contentHash);
}
