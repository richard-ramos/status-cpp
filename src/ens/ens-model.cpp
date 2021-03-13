#include "ens-model.hpp"
#include "constants.hpp"
#include "ens-utils.hpp"
#include "settings.hpp"
#include "status.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

ENSModel::ENSModel(QObject* parent)
	: QAbstractListModel(parent)
	, m_names(Settings::instance()->usernames())
{ }

QHash<int, QByteArray> ENSModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Name] = "username";
	return roles;
}

int ENSModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_names.size();
}

QVariant ENSModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	QString name = m_names[index.row()];

	switch(role)
	{
	case Name: return QVariant(name);
	}

	return QVariant();
}

void ENSModel::push(QString name)
{
	insert(name);
}

void ENSModel::insert(QString name)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_names << name;
	endInsertRows();
}

void ENSModel::connect(QString ensUsername)
{
	auto usernames = Settings::instance()->usernames();
	usernames << ensUsername;
	Settings::instance()->setUsernames(usernames);
}

QString ENSModel::formatUsername(QString username, bool isStatus)
{
	if(isStatus)
	{
		return username + Constants::StatusDomain;
	}
	else
	{
		return username;
	}
}

void ENSModel::connectOwnedUsername(QString username, bool isStatus)
{
	QString ensUsername(formatUsername(username, isStatus));
	insert(ensUsername);
	connect(ensUsername);
}

void ENSModel::validate(QString username, bool isStatus, const QJSValue& callback)
{
	auto* watcher = new QFutureWatcher<QString>(this);
	QObject::connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, callback]() {
		QString result = watcher->result();
		QJSValue cbCopy(callback); // needed as callback is captured as const
		QJSEngine* engine = qjsEngine(this);
		cbCopy.call(QJSValueList{engine->toScriptValue(result)});
		watcher->deleteLater();
	});
	watcher->setFuture(QtConcurrent::run(this, &ENSModel::validate, username, isStatus));
}

QString ENSModel::validate(QString ens, bool isStatus)
{
	QString username(formatUsername(ens, isStatus));

	QString output;

	if(Settings::instance()->usernames().indexOf(username) > -1)
	{
		output = "already-connected";
	}
	else
	{
		QScopedPointer<Ens::Utils> ensUtils(new Ens::Utils());
		QString ownerAddr = ensUtils->owner(username); // TODO: extract to static function
		if(ownerAddr == "" && isStatus)
		{
			return "available";
		}
		else
		{
			QString userPubKey = Settings::instance()->publicKey();
			QString userWallet = Settings::instance()->walletRootAddress();

			QString pubkey = ensUtils->pubKey(username); // TODO: extract to static function

			if(ownerAddr != "")
			{
				if(pubkey == "" && ownerAddr == userWallet)
				{
					//  "Continuing will connect this username with your chat key."
					output = "owned";
				}
				else if(pubkey == userPubKey)
				{
					output = "connected";
				}
				else if(ownerAddr == userWallet)
				{
					// "Continuing will require a transaction to connect the username with your current chat key."
					output = "connected-different-key";
				}
				else
				{
					output = "taken";
				}
			}
			else
			{
				output = "taken";
			}
		}
	}

	return output;
}