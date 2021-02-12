#pragma once

#include <QAbstractListModel>

struct NodeAccount
{
	QString keyUid;
	QString name;
	QString identicon;
	QString image;
	QString keycardPairing;
	int timestamp;
};

class LoginModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(QString selectedAccount READ selectedAccount NOTIFY selectedAccountChanged)
	Q_PROPERTY(QVariantMap currentAccount READ currentAccount NOTIFY selectedAccountChanged)

public:

    static QString path;
	enum LoginRoles
	{
		Id = Qt::UserRole + 1,
		PublicKey = Qt::UserRole + 2,
		Image = Qt::UserRole + 3,
		Name = Qt::UserRole + 4
	};

	explicit LoginModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void reload();
	Q_INVOKABLE QString getAccountId(int index);
	Q_INVOKABLE void login(QString password);
	Q_INVOKABLE void setSelectedAccount(int index);

	QString selectedAccount() const;
	QVariantMap currentAccount() const;

signals:
	void selectedAccountChanged(QString);
	void loginError(QString);

private:
	QVector<NodeAccount> mData;
	QString m_selectedAccount;
};
