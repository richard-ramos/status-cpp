#include "custom-networks-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

CustomNetworksModel::CustomNetworksModel(QObject* parent)
	: QAbstractListModel(parent)
{
	loadCustomNetworks();
	QObject::connect(this, &CustomNetworksModel::networkLoaded, this, &CustomNetworksModel::push);
}

QHash<int, QByteArray> CustomNetworksModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Id] = "networkId";
	roles[Name] = "name";
	return roles;
}

int CustomNetworksModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_customNetworks.size();
}

QVariant CustomNetworksModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	CustomNetwork network = m_customNetworks[index.row()];

	switch(role)
	{
	case Id: return QVariant(network.id);
	case Name: return QVariant(network.name);
	}

	return QVariant();
}

void CustomNetworksModel::loadCustomNetworks()
{
	QtConcurrent::run([=] {
		QJsonArray networks = Settings::instance()->networks();
		QVector<QString> defaultNetworks = {"mainnet_rpc", "testnet_rpc", "rinkeby_rpc", "goerli_rpc", "xdai_rpc", "poa_rpc"};
		foreach(const QJsonValue& value, networks)
		{
			const QJsonObject obj = value.toObject();
			if(defaultNetworks.contains(obj["id"].toString()))
				continue;
			CustomNetwork c{obj["id"].toString(), obj["name"].toString()};
			emit networkLoaded(c);
		}
	});
}

void CustomNetworksModel::push(CustomNetwork network)
{
	insert(network);
}

void CustomNetworksModel::insert(CustomNetwork network)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_customNetworks << network;
	endInsertRows();
}

void CustomNetworksModel::add(const QString& name, const QString& url, int networkId, const QString& networkType)
{
	QJsonArray networks = Settings::instance()->networks();

	CustomNetwork n = {QUuid::createUuid().toString(QUuid::WithoutBraces), name};

	QJsonObject newNetwork{{"id", n.id},
						   {"name", name},
						   {"config",
							QJsonObject{{"NetworkId", networkId},
										{"DataDir", "/ethereum/" + networkType},
										{"UpstreamConfig", QJsonObject{{"Enabled", true}, {"URL", url}}}}}};
	networks << newNetwork;

	Settings::instance()->setNetworks(networks);

	insert(n);
}
