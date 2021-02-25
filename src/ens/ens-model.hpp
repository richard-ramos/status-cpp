#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QHash>
#include <QJSValue>

class ENSModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum ENSRoles
	{
		Name = Qt::UserRole + 1
		// TODO: is Pending
	};

	explicit ENSModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(QString name);
	Q_INVOKABLE void connect(QString ensUsername);
	Q_INVOKABLE void connectOwnedUsername(QString username, bool isStatus);
	Q_INVOKABLE QString validate(QString username, bool isStatus);
	Q_INVOKABLE void validate(QString username, bool isStatus, const QJSValue& callback);

signals:
    void nameLoaded(QString name);

private:
	void loadENSnames();
	void insert(QString name);
	QString formatUsername(QString username, bool isStatus);

	QVector<QString> m_names;
};
