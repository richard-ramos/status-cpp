#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QHash>

struct CustomNetwork
{
	QString id;
	QString name;
};

class CustomNetworksModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum CustomNetworkRoles
	{
		Id = Qt::UserRole + 1,
		Name = Qt::UserRole + 2
	};

	explicit CustomNetworksModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(CustomNetwork network);
    Q_INVOKABLE void add(const QString& name, const QString& url, int networkId, const QString& networkType);

signals:
    void networkLoaded(CustomNetwork network);

private:
	void loadCustomNetworks();
	void insert(CustomNetwork network);

	QVector<CustomNetwork> m_customNetworks;
};
