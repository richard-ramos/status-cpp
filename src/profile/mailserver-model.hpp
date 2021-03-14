#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonValue>
#include <QVector>
#include <QTimer>
#include "mailserver-cycle.hpp"

struct Mailserver
{
	QString name;
	QString endpoint;
};

class MailserverModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum MailserverRoles
	{
		Name = Qt::UserRole + 1,
		Endpoint = Qt::UserRole + 2
	};

	explicit MailserverModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(Mailserver mailserver);
	Q_INVOKABLE void startMailserverCycle();
    
//	Q_INVOKABLE void add(QString name, QString endpoint);

	MailserverCycle* getCycle();

signals:
	void mailserverRequestSent();
	void mailserverLoaded(Mailserver mailserver);

private:
	void loadMailservers();
	void insert(Mailserver mailserver);

	QTimer* timer;
	MailserverCycle* mailserverCycle;
	QVector<Mailserver> m_mailservers;
};
