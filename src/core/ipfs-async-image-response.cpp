#include "ipfs-async-image-response.hpp"
#include <QByteArray>
#include <QNetworkDiskCache>
#include <QObject>
#include <QQuickTextureFactory>

IPFSAsyncImageResponse::IPFSAsyncImageResponse(QNetworkRequest req, QSize const& reqSize, QString const& cacheDir)
{
	m_imageLoader = new QNetworkAccessManager();
	QNetworkDiskCache* cache = new QNetworkDiskCache(m_imageLoader);
	cache->setCacheDirectory(cacheDir);
	m_imageLoader->setCache(cache);
	m_reply = m_imageLoader->get(req);
	m_requestedSize = reqSize;
	connect(m_reply, &QNetworkReply::finished, this, &IPFSAsyncImageResponse::onResponseFinished);
}

void IPFSAsyncImageResponse::onResponseFinished()
{
	QByteArray myImageData = m_reply->readAll();
	m_resultImage = QImage::fromData(myImageData);
	if(m_requestedSize.isValid())
	{
		m_resultImage = m_resultImage.scaled(m_requestedSize);
	}
	emit finished();
}

QQuickTextureFactory* IPFSAsyncImageResponse::textureFactory() const
{
	return QQuickTextureFactory::textureFactoryForImage(m_resultImage);
}
