#include "stickerpack.hpp"
#include "constants.hpp"
#include "edn.hpp"
#include "utils.hpp"
#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QString>
#include <QVector>

StickerPack::StickerPack(
	int id, QStringList category, QString address, bool mintable, QString timestamp, QString price, QString contentHash, QObject* parent)
	: QObject(parent)
	, m_id(id)
	, m_category(category)
	, m_address(address)
	, m_mintable(mintable)
	, m_timestamp(timestamp)
	, m_price(price)
	, m_contentHash(contentHash)
{ }

void StickerPack::loadContent(QNetworkAccessManager* manager)
{
	QUrl url(Constants::StatusIPFS + m_contentHash);
	QNetworkRequest request(url);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
	QNetworkReply* netReply = manager->get(request);

	// Wait
	QEventLoop loop;
	connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
	connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));

	loop.exec();

	if(netReply->error() != QNetworkReply::NoError)
	{
		qWarning() << "Error requesting StickerPack content:" << m_contentHash << netReply->error();
		return;
	}

	edn::EdnNode stickerPackDefinition = edn::read(netReply->readAll().toStdString());

	delete netReply;

	for(edn::EdnNode node1 : stickerPackDefinition.values)
	{
		if(node1.type == edn::NodeType::EdnMap)
		{
			QVector<edn::EdnNode> nodeVector(node1.values.begin(), node1.values.end());
			for(int i = 0; i < nodeVector.count() / 2; i++)
			{
				if(nodeVector[i * 2].type == edn::NodeType::EdnKeyword && nodeVector[i * 2 + 1].type == edn::NodeType::EdnString &&
				   nodeVector[i * 2].value == ":name")
				{
					m_name = QString::fromStdString(nodeVector[i * 2 + 1].value);
				}
				else if(nodeVector[i * 2].type == edn::NodeType::EdnKeyword && nodeVector[i * 2 + 1].type == edn::NodeType::EdnString &&
						nodeVector[i * 2].value == ":author")
				{
					m_author = QString::fromStdString(nodeVector[i * 2 + 1].value);
				}
				else if(nodeVector[i * 2].type == edn::NodeType::EdnKeyword && nodeVector[i * 2 + 1].type == edn::NodeType::EdnString &&
						nodeVector[i * 2].value == ":thumbnail")
				{
					m_thumbnail = Utils::decodeHash(QString::fromStdString(nodeVector[i * 2 + 1].value));
				}
				else if(nodeVector[i * 2].type == edn::NodeType::EdnKeyword && nodeVector[i * 2 + 1].type == edn::NodeType::EdnString &&
						nodeVector[i * 2].value == ":preview")
				{
					m_preview = Utils::decodeHash(QString::fromStdString(nodeVector[i * 2 + 1].value));
				}
				else if(nodeVector[i * 2].type == edn::NodeType::EdnKeyword && nodeVector[i * 2 + 1].type == edn::NodeType::EdnVector &&
						nodeVector[i * 2].value == ":stickers")
				{
					QStringList stickers;
					QVector<edn::EdnNode> stickersEdn(nodeVector[i * 2 + 1].values.begin(), nodeVector[i * 2 + 1].values.end());
					for(int j = 0; j < stickersEdn.count(); j++)
					{
						QVector<edn::EdnNode> stickerHash(stickersEdn[j].values.begin(), stickersEdn[j].values.end());
						if(stickerHash[0].type == edn::NodeType::EdnKeyword && stickerHash[0].value == ":hash" &&
						   stickerHash[1].type == edn::NodeType::EdnString)
						{
							stickers << QString::fromStdString(stickerHash[1].value);
						}
					}
					m_stickers = stickers;
				}
			}
		}
	}
}