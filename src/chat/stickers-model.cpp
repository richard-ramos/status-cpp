#include "stickers-model.hpp"
#include "constants.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "stickerpack-utils.hpp"
#include "stickerpack.hpp"
#include "uint256_t.h"
#include "utils.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QQmlApplicationEngine>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QNetworkDiskCache>
#include <algorithm>
#include <array>

StickerPacksModel::StickerPacksModel(QObject* parent)
	: QAbstractListModel(parent)
{
	loadStickerPacks();
	QObject::connect(this, &StickerPacksModel::stickerPackLoaded, this, &StickerPacksModel::push);
}

QHash<int, QByteArray> StickerPacksModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Author] = "author";
	roles[Id] = "packId";
	roles[Name] = "name";
	roles[Price] = "price";
	roles[Preview] = "preview";
	roles[Stickers] = "stickers";
	roles[Thumbnail] = "thumbnail";
	roles[Installed] = "installed";
	roles[Bought] = "bought";
	roles[Pending] = "pending";
	return roles;
}

int StickerPacksModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_stickerPacks.size();
}

QVariant StickerPacksModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	StickerPack* pack = m_stickerPacks[index.row()];

	switch(role)
	{
	case Author: return QVariant(pack->get_author());
	case Id: return QVariant(pack->get_id());
	case Name: return QVariant(pack->get_name());
	case Preview: return QVariant(pack->get_preview());
	case Thumbnail: return QVariant(pack->get_thumbnail());
	case Price: return QVariant(Utils::wei2Token(pack->get_price()));
	case Stickers: return QVariant::fromValue(pack->get_stickers());
	case Installed: return QVariant(false);
	case Bought: return QVariant(false); // TODO:
	case Pending: return QVariant(false); // TODO:
	}

	return QVariant();
}

void StickerPacksModel::loadStickerPacks()
{
	QtConcurrent::run([=] {
		// TODO: should this be part of the StickerPacksModel object?
		QNetworkAccessManager* manager = new QNetworkAccessManager();
		QNetworkDiskCache *diskCache = new QNetworkDiskCache();
		diskCache->setCacheDirectory(Constants::cachePath("/stickers/network"));
		manager->setCache(diskCache);

		int numPacks = StickerPackUtils::getPackCount();
		for(int i = 0; i < numPacks; i++)
		{
			StickerPack* stickerPack = StickerPackUtils::getPackData(i);
			stickerPack->loadContent(manager);
			stickerPack->moveToThread(QApplication::instance()->thread());
			emit stickerPackLoaded(stickerPack);
		}

		delete manager;
	});
}

void StickerPacksModel::reloadStickers(){
	beginResetModel();
	for(int i = 0; i < m_stickerPacks.count(); i++){
		delete m_stickerPacks[i];
	}
	m_stickerPacks.clear();
	endResetModel();
	loadStickerPacks();
}

void StickerPacksModel::install(int packId)
{
	foreach(StickerPack* sticker, m_stickerPacks)
	{
		if(sticker->get_id() == packId)
		{
			QJsonObject installedStickerPacks = Settings::instance()->installedStickerPacks();
			installedStickerPacks[QString::number(packId)] =
				QJsonObject{{"thumbnail", sticker->get_thumbnail()}, {"stickers", QJsonArray::fromStringList(sticker->get_stickers())}};
			Settings::instance()->setInstalledStickerPacks(installedStickerPacks);
			// TODO: change installed to true
		}
	}
}

void StickerPacksModel::push(StickerPack* pack)
{
	insert(pack);
}

void StickerPacksModel::insert(StickerPack* pack)
{
	QQmlApplicationEngine::setObjectOwnership(pack, QQmlApplicationEngine::CppOwnership);
	pack->setParent(this);
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_stickerPacks << pack;
	endInsertRows();
}