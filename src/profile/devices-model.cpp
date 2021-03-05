#include "devices-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QSysInfo>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

DevicesModel::DevicesModel(QObject* parent)
	: QAbstractListModel(parent)
{
	loadDevices();
	QObject::connect(this, &DevicesModel::deviceLoaded, this, &DevicesModel::push);
	QObject::connect(Status::instance(), &Status::message, this, &DevicesModel::update);
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[InstallationId] = "installationId";
	roles[Name] = "name";
	roles[IsUserDevice] = "isUserDevice";
	roles[IsEnabled] = "isEnabled";
	roles[Timestamp] = "timestamp";

	return roles;
}

int DevicesModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_devices.size();
}

QVariant DevicesModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Device device = m_devices[index.row()];

	switch(role)
	{
	case InstallationId: return QVariant(device.installationId);
	case Name: return QVariant(device.name);
	case IsUserDevice: return QVariant(device.installationId == Settings::instance()->installationId());
	case IsEnabled: return QVariant(device.enabled);
	case Timestamp: return QVariant(device.timestamp);
	}

	return QVariant();
}

void DevicesModel::loadDevices()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("wakuext_getOurInstallations", QJsonArray{}.toVariantList()).toJsonObject();
		foreach(QJsonValue deviceJson, response["result"].toArray())
		{
			const QJsonObject obj = deviceJson.toObject();
			Device d{.installationId = obj["id"].toString(),
					 .name = obj["metadata"]["name"].toString(),
					 .timestamp = obj["timestamp"].toString(),
					 .enabled = obj["enabled"].toBool()};
			emit deviceLoaded(d);
		}
	});
}

void DevicesModel::push(Device device)
{
	insert(device);
	if(device.installationId == Settings::instance()->installationId())
	{
		deviceSetupChanged();
	}
}

bool DevicesModel::isDeviceSetup()
{
	int index = m_devices.indexOf(Device{.installationId = Settings::instance()->installationId()});
	return index > -1 && !m_devices[index].name.isEmpty();
}

void DevicesModel::insert(Device device)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_devices << device;
	endInsertRows();
}

void DevicesModel::setName(QString name)
{
	QString deviceType = QSysInfo::productType();
	const auto response = Status::instance()->callPrivateRPC(
		"wakuext_setInstallationMetadata",
		QJsonArray{Settings::instance()->installationId(), QJsonObject{{"name", name}, {"deviceType", deviceType}}}.toVariantList());
	emit deviceSetupChanged();
	int index = m_devices.indexOf(Device{.installationId = Settings::instance()->installationId()});
	if(index > -1)
	{
		m_devices[index].name = name;
		QModelIndex idx = createIndex(index, 0);
		dataChanged(idx, idx);
	}
}

void DevicesModel::syncAll()
{
	QString photoPath = ""; // TODO:
	const auto response =
		Status::instance()->callPrivateRPC("wakuext_syncDevices", QJsonArray{Settings::instance()->preferredName(), photoPath}.toVariantList());
}

void DevicesModel::advertise()
{
	const auto response = Status::instance()->callPrivateRPC("wakuext_sendPairInstallation", QJsonArray{}.toVariantList());
}

void DevicesModel::enableInstallation(QString installationId, bool enabled)
{
	QString methodName = enabled ? "wakuext_enableInstallation" : "wakuext_disableInstallation";
	Status::instance()->callPrivateRPC(methodName, QJsonArray{installationId}.toVariantList());
}

void DevicesModel::update(QJsonValue updates)
{
	foreach(QJsonValue deviceJson, updates["installations"].toArray())
	{
		const QJsonObject obj = deviceJson.toObject();
		Device d{.installationId = obj["id"].toString(),
				 .name = obj["metadata"]["name"].toString(),
				 .timestamp = obj["timestamp"].toString(),
				 .enabled = obj["enabled"].toBool()};
		emit deviceLoaded(d);
	}
}