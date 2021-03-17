#pragma once

#include "mailserver-cycle.hpp"
#include <QAbstractListModel>
#include <QHash>
#include <QJsonValue>
#include <QTimer>
#include <QVector>

struct Mailserver
{
	QString id;
	QString name;
	QString endpoint;

	Q_PROPERTY(QString id MEMBER id)
	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QString endpoint MEMBER endpoint)
	Q_GADGET
};

class MailserverModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum MailserverRoles
	{
		Id = Qt::UserRole + 1,
		Name = Qt::UserRole + 2,
		Endpoint = Qt::UserRole + 3
	};

	explicit MailserverModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(Mailserver mailserver);
	Q_INVOKABLE void startMailserverCycle();
	Q_INVOKABLE void enableAutomaticSelection(bool enable);
	Q_INVOKABLE void pinMailserver(QString endpoint);
	Q_INVOKABLE void add(QString name, QString endpoint);

	Q_PROPERTY(MailserverCycle* cycle READ getCycle)
	MailserverCycle* getCycle();
	void loadCustomMailservers();

	Q_PROPERTY(Mailserver ActiveMailserver READ getActiveMailserver NOTIFY activeMailserverChanged)
	Mailserver getActiveMailserver();

signals:
	void mailserverRequestSent();
	void mailserverLoaded(Mailserver mailserver);
	void activeMailserverChanged();

private:
	void loadMailservers();
	void insert(Mailserver mailserver);

	QTimer* timer;
	MailserverCycle* mailserverCycle;
	QVector<Mailserver> m_mailservers;
};
