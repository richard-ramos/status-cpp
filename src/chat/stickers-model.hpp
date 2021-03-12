#pragma once

#include "stickerpack.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QQmlHelpers>
#include <QReadWriteLock>
#include <QVector>

class StickerPacksModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum StickerPackRoles
	{
		Author = Qt::UserRole + 1,
		Id = Qt::UserRole + 2,
		Name = Qt::UserRole + 3,
		Price = Qt::UserRole + 4,
		Preview = Qt::UserRole + 5,
		Stickers = Qt::UserRole + 6,
		Thumbnail = Qt::UserRole + 7,
		Installed = Qt::UserRole + 8,
		Bought = Qt::UserRole + 9,
		Pending = Qt::UserRole + 10

	};

	explicit StickerPacksModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(StickerPack* pack);
	Q_INVOKABLE void reloadStickers();
	Q_INVOKABLE void install(int packId);
	Q_INVOKABLE void uninstall(int packId);

signals:
	void stickerPackLoaded(StickerPack* pack);

private:
	void loadStickerPacks();
	void insert(StickerPack* pack);

	// TODO: purchase

	QVector<StickerPack*> m_stickerPacks;

	mutable QReadWriteLock m_installedStickersLock;
	QSet<int> m_installedStickers;
};
