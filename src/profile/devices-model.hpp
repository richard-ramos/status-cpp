#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonValue>
#include <QVector>

struct Device
{
	QString installationId;
	QString name;
    QString timestamp;
    bool enabled;


bool operator==(const Device& c)
{
	return installationId == c.installationId;
}

};

class DevicesModel : public QAbstractListModel
{
	Q_OBJECT

    Q_PROPERTY(bool isSetup READ isDeviceSetup NOTIFY deviceSetupChanged)


public:
	enum DevicesRoles
	{
		InstallationId = Qt::UserRole + 1,
		Name = Qt::UserRole + 2,
		IsUserDevice = Qt::UserRole + 3,
		IsEnabled = Qt::UserRole + 4,
		Timestamp = Qt::UserRole + 5
	};

	explicit DevicesModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(Device device);

    bool isDeviceSetup();
    
	Q_INVOKABLE void setName(QString deviceName);
	Q_INVOKABLE void advertise();
	Q_INVOKABLE void syncAll();
	Q_INVOKABLE void enableInstallation(QString installationId, bool enabled);
	
	Q_INVOKABLE void update(QJsonValue updates);

signals:
	void deviceLoaded(Device device);
    void deviceSetupChanged();

private:
	void loadDevices();
	void insert(Device device);

	QVector<Device> m_devices;
};
