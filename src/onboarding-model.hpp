#ifndef ONBOARDINGMODEL_H
#define ONBOARDINGMODEL_H

#include <QAbstractListModel>

const QString pathWalletRoot("m/44'/60'/0'/0");
const QString pathEip1581("m/43'/60'/1581'");
const QString pathDefaultWallet(pathWalletRoot + "/0");
const QString pathWhisper(pathEip1581  + "/0'/0");
const QString DEFAULT_NETWORK_NAME = "mainnet_rpc";

struct DerivedKey {
    QString publicKey; 
    QString address;
};

struct GeneratedAccount
{
    QString id;
    QString address; 
    QString keyUid;
    QString mnemonic;
    QString publicKey;
    std::map<QString, DerivedKey> derivedKeys;
};

class OnboardingModel : public QAbstractListModel
{
Q_OBJECT
public:
    enum OnboardingRoles {
        Id = Qt::UserRole + 1,
        PublicKey = Qt::UserRole + 2,
        Identicon = Qt::UserRole + 3,
        Name = Qt::UserRole + 4
    };

    explicit OnboardingModel(QObject * parent = nullptr);

    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex &index, int role) const;


    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void populate();
    Q_INVOKABLE void setup(QString accountId, QString password);
    Q_INVOKABLE QString getAccountId(int index);
    Q_INVOKABLE QString validateMnemonic(QString mnemonic);
    Q_INVOKABLE QString importMnemonic(QString mnemonic);

    Q_INVOKABLE QVariantMap get(int index) const;


signals:
  void accountSaved(bool result);


private:
    QVariant getModelData(const GeneratedAccount& acc, int role) const;
    QVector<GeneratedAccount> mData;
};

#endif // ONBOARDINGMODEL_H