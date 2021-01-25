#ifndef LOGINMODEL_H
#define LOGINMODEL_H

#include <QAbstractListModel>

struct NodeAccount
{
    QString keyUid;
    QString name;
    QString identicon; 
    QString keycardPairing;
    int timestamp;
};

class LoginModel : public QAbstractListModel
{
Q_OBJECT
Q_PROPERTY(QString selectedAccount READ selectedAccount NOTIFY selectedAccountChanged)
Q_PROPERTY(QVariantMap currentAccount READ currentAccount NOTIFY selectedAccountChanged)

public:
    enum LoginRoles {
        Id = Qt::UserRole + 1,
        PublicKey = Qt::UserRole + 2,
        Identicon = Qt::UserRole + 3,
        Name = Qt::UserRole + 4
    };

    explicit LoginModel(QObject * parent = nullptr);

    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE QString getAccountId(int index);
    Q_INVOKABLE void login(QString password);

    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void setSelectedAccount(int index);
    
    QString selectedAccount() const;


    QVariantMap currentAccount() const;


signals:
    void selectedAccountChanged(QString);


private:
    QVector<NodeAccount> mData;
    QString m_selectedAccount;
};

#endif // LOGINMODEL_H