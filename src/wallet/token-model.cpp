#include "token-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlApplicationEngine>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>

TokenModel::TokenModel(QObject* parent)
	: QAbstractListModel(parent)
{
	QObject::connect(this, &TokenModel::tokenLoaded, this, &TokenModel::push);
	loadTokens();
	loadCustomTokens();
}

QHash<int, QByteArray> TokenModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Name] = "name";
	roles[Symbol] = "symbol";
	roles[HasIcon] = "hasIcon";
	roles[Address] = "address";
	roles[Decimals] = "decimals";
	roles[IsCustom] = "isCustom";
	roles[IsVisible] = "isTokenVisible";
	return roles;
}

int TokenModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_tokens.size();
}

QVariant TokenModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}

	Token t = m_tokens[index.row()];

	switch(role)
	{
	case Name: return QVariant(t.name);
	case Symbol: return QVariant(t.symbol);
	case IsCustom: return QVariant(t.isCustom);
	case Address: return QVariant(t.address);
	case Decimals: return QVariant(t.decimals);
	case HasIcon: return QVariant(t.hasIcon);
	case IsVisible: return QVariant(t.isVisible);
	}

	return QVariant();
}

void TokenModel::loadTokens()
{
	QtConcurrent::run([=] {
		QFile defaultTokens(":/resources/default-tokens.json");
		defaultTokens.open(QIODevice::ReadOnly);
		QString defaultTokensContent = defaultTokens.readAll();
		QJsonArray defaultTokensJson = QJsonDocument::fromJson(defaultTokensContent.toUtf8()).array();
		QString currentNetwork = Settings::instance()->currentNetwork();

		QVector<QString> visibleTokens = Settings::instance()->visibleTokens();

		bool tokenVisibilitySet = Settings::instance()->tokenVisibilitySet();

		foreach(const QJsonValue& value, defaultTokensJson)
		{
			const QJsonObject obj = value.toObject();
			QString tokenAddress = obj["address"].toString();
			QString tokenSymbol = obj["symbol"].toString();

			if(obj["network"].toString() != currentNetwork) continue;

			bool isVisible = visibleTokens.contains(tokenSymbol);
			if(!tokenVisibilitySet)
			{
				// Enable SNT/STT
				if((tokenSymbol == "SNT" && currentNetwork == "mainnet_rpc") || (tokenSymbol == "STT" && currentNetwork != "mainnet_rpc"))
				{
					isVisible = true;
					visibleTokens <<  (currentNetwork == "mainnet_rpc" ? "SNT" : "STT");
					Settings::instance()->setVisibleTokens(visibleTokens);
				}
			}

			Token t{.name = obj["name"].toString(),
					.symbol = obj["symbol"].toString(),
					.hasIcon = obj["hasIcon"].toBool(),
					.decimals = obj["decimals"].toInt(),
					.isCustom = false,
					.address = tokenAddress,
					.isVisible = isVisible};

			emit tokenLoaded(t);
		}
	});
}

void TokenModel::loadCustomTokens()
{
	QtConcurrent::run([=] {
		const auto response = Status::instance()->callPrivateRPC("wallet_getCustomTokens", QJsonArray{}.toVariantList()).toJsonObject();

		if(response["result"].isNull()) return;

		QVector<QString> visibleTokens = Settings::instance()->visibleTokens();

		foreach(const QJsonValue& value, response["result"].toArray())
		{
			const QJsonObject obj = value.toObject();
			QString tokenAddress = obj["address"].toString();
			QString tokenSymbol = obj["symbol"].toString();

			bool isVisible = visibleTokens.contains(tokenSymbol);

			Token t{.name = obj["name"].toString(),
					.symbol = obj["symbol"].toString(),
					.hasIcon = obj["hasIcon"].toBool(),
					.decimals = obj["decimals"].toInt(),
					.isCustom = true,
					.address = tokenAddress,
					.isVisible = isVisible};

			emit tokenLoaded(t);
		}
	});
}

void TokenModel::remove(QString tokenSymbol, QString tokenAddress)
{
	const auto response = Status::instance()->callPrivateRPC("wallet_deleteCustomToken", QJsonArray{tokenAddress}.toVariantList()).toJsonObject();

	// TODO: error handling

	QVector<QString> visibleTokens = Settings::instance()->visibleTokens();
	int idx = visibleTokens.indexOf(tokenSymbol);
	if(idx > -1)
	{
		visibleTokens.remove(idx);
	}
	Settings::instance()->setVisibleTokens(visibleTokens);

	for(int i = 0; i < m_tokens.size(); i++)
	{
		if(m_tokens[i].address == tokenAddress && m_tokens[i].isCustom == true)
		{
			beginRemoveRows(QModelIndex(), i, i);
			m_tokensMap.remove(m_tokens[i].symbol);
			m_tokens.remove(i);
			endRemoveRows();
			break;
		}
	}
}

void TokenModel::toggle(QString tokenSymbol, QString tokenAddress, bool isCustom)
{
	QVector<QString> visibleTokens = Settings::instance()->visibleTokens();

	bool visible = false;
	int idx = visibleTokens.indexOf(tokenSymbol);
	if(idx > -1)
	{
		visibleTokens.remove(idx);
	}
	else
	{
		visibleTokens << tokenSymbol;
		visible = true;
	}

	Settings::instance()->setVisibleTokens(visibleTokens);

	for(int i = 0; i < m_tokens.size(); i++)
	{
		if(m_tokens[i].address == tokenAddress && m_tokens[i].isCustom == isCustom)
		{
			QModelIndex idx = createIndex(i, 0);
			m_tokens[i].isVisible = visible;
			dataChanged(idx, idx);
			break;
		}
	}
}

QString TokenModel::add(QString address, QString name, QString symbol, int decimals)
{
	for(const Token& token : m_tokens)
	{
		if(token.address == address)
		{
			return "token-exists";
		}
	}

	const auto response =
		Status::instance()
			->callPrivateRPC("wallet_addCustomToken",
							 QJsonArray{QJsonObject{{"address", address}, {"name", name}, {"symbol", symbol}, {"decimals", decimals}, {"color", ""}}}
								 .toVariantList())
			.toJsonObject();

	// TODO: rpc error handling

	Token t{.name = name, .symbol = symbol, .hasIcon = false, .decimals = decimals, .isCustom = true, .address = address, .isVisible = false};
	emit tokenLoaded(t);

	return "";
}

void TokenModel::push(Token token)
{
	insert(token);
}

void TokenModel::insert(Token token)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_tokens << token;
	m_tokensMap[token.symbol] = token;
	endInsertRows();
}

std::optional<Token> TokenModel::token(QString tokenSymbol){
	if(m_tokensMap.contains(tokenSymbol)){
		return m_tokensMap[tokenSymbol];
	}
	return {};
}

