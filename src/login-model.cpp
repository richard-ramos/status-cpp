#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QCryptographicHash>
#include <QUuid>
#include <QRandomGenerator>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <algorithm>
#include <array>

#include "login-model.hpp"
#include "libstatus.h"
#include "utils.hpp"
#include "status.hpp"
#include "constants.hpp"

QVector<NodeAccount> openAccounts()
{
    QString fullDirPath = QCoreApplication::applicationDirPath() + Constants::DataDir; // TODO: set correct path
    const char* result { OpenAccounts(fullDirPath.toUtf8().data()) };
    QJsonArray multiAccounts = QJsonDocument::fromJson(result).array();

    QVector<NodeAccount> vector;
    foreach (const QJsonValue & value, multiAccounts) {
        const QJsonObject obj = value.toObject();
        NodeAccount acc = {};
        acc.name = obj["name"].toString();
        acc.identicon = obj["identicon"].toString();
        if(obj["images"].isArray()){
            foreach(const QJsonValue image, obj["images"].toArray()){
                if(image["type"].toString() == "thumbnail"){
                    acc.image = image["uri"].toString();
                }
            }
        }

        acc.keyUid = obj["key-uid"].toString();
        acc.keycardPairing = obj["keycard-pairing"].toString();
        acc.timestamp = obj["timestamp"].toInt();
        vector.append(acc);
    }
    return vector;
}

LoginModel::LoginModel(QObject * parent): QAbstractListModel(parent)
{
    mData << openAccounts();
    if(mData.count()){
        m_selectedAccount = mData[0].keyUid;
    }

    QObject::connect(Status::instance(), &Status::logout, this, &LoginModel::reload);
}


void LoginModel::reload()
{
    beginResetModel();
    mData.clear();
    mData << openAccounts();
    m_selectedAccount = mData[0].keyUid;
    emit selectedAccountChanged(m_selectedAccount);
    endResetModel();
}


QHash<int, QByteArray> LoginModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id] = "id";
    roles[PublicKey] = "publicKey";
    roles[Image] = "image";
    roles[Name] = "name";
    return roles;
}


QString LoginModel::getAccountId(int index)
{
    return mData[index].keyUid; 
}


int LoginModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return mData.size();
}


QVariant LoginModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch (role)
    {
        case Id: return QVariant(mData[index.row()].keyUid);
        case Image: return QVariant(mData[index.row()].image == "" ? mData[index.row()].identicon : mData[index.row()].image);
        case Name: return QVariant(mData[index.row()].name);
        case PublicKey: return QVariant(mData[index.row()].keyUid);
    }

    return QVariant();
}


void LoginModel::setSelectedAccount(int index)
{
    m_selectedAccount = mData[index].keyUid;
    emit selectedAccountChanged(m_selectedAccount);
}


QJsonObject getAccountData(NodeAccount* account)
{
    return QJsonObject {
         {"name", account->name},
         {"photo-path", account->identicon},
         {"identicon", account->identicon},
         {"key-uid", account->keyUid},
    };
}


QJsonObject loginAccount(NodeAccount* nodeAccount, QString password) {
    QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256)); 
    QJsonObject accountData( getAccountData(nodeAccount) );
    const char* result { Login(Utils::jsonToStr(accountData).toUtf8().data(), hashedPassword.toUtf8().data()) };
    return QJsonDocument::fromJson(result).object();  
    // TODO: clear password
}


void LoginModel::login(QString password)
{
    auto nodeAccount = std::find_if(mData.begin(), mData.end(), [this](const NodeAccount& m) -> bool { return m.keyUid == m_selectedAccount; });
    if(nodeAccount != mData.end()){
        QtConcurrent::run([=]{
            QJsonObject response = loginAccount(nodeAccount, password);
            QString error = response["error"].toString();
            if(error != ""){
                qInfo() << "Login error: " << error;
                emit loginError(error);
            }
        });
    } else {
        qInfo() << "Login error: Account not found";
        emit loginError("Account not found");
    }
}

QString LoginModel::selectedAccount() const
{ 
    return m_selectedAccount;
}


QVariantMap LoginModel::currentAccount() const
{
    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    QVariantMap res;

    for(int row = 0; row < mData.count(); row++){
        if(mData[row].keyUid != m_selectedAccount) continue;
        QModelIndex idx = index(row, 0);
        while(i.hasNext()) {
            i.next();
            QVariant data = idx.data(i.key());
            res[i.value()] = data;
        }
    }

    return res;    
}