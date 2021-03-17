#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

struct Token
{
	QString name;
	QString symbol;
	bool hasIcon;
	int decimals;
	bool isCustom;
    QString address;
    bool isVisible;
};

class TokenModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum TokenRoles
	{
		Name = Qt::UserRole + 1,
		Symbol = Qt::UserRole + 2,
		HasIcon = Qt::UserRole + 3,
		Address = Qt::UserRole + 4,
		Decimals = Qt::UserRole + 5,
		IsCustom = Qt::UserRole + 6,
        IsVisible = Qt::UserRole + 7
	};

	explicit TokenModel(QObject* parent = nullptr);

	QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE void push(Token token);
	Q_INVOKABLE QString add(QString address, QString name, QString symbol, int decimals);
	Q_INVOKABLE void remove(QString address);
    Q_INVOKABLE void toggle(QString tokenAddress, bool isCustom);


signals:
	void tokenLoaded(Token token);

private:
	void loadTokens();
	void loadCustomTokens();
	void insert(Token token);

	QVector<Token> m_tokens;
};
