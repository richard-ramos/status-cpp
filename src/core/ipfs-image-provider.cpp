#include "ipfs-image-provider.hpp"
#include "ipfs-async-image-response.hpp"
#include <QNetworkRequest>
#include <QQuickAsyncImageProvider>
#include <QString>

IPFSAsyncImageProvider::IPFSAsyncImageProvider(QString ipfsTmpDir, QString ipfsGateway)
	: m_gateway(ipfsGateway)
	, m_cacheDir(ipfsTmpDir)
{ }

QQuickImageResponse* IPFSAsyncImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
	QNetworkRequest ipfsNetRequest(QUrl(m_gateway + id));
	ipfsNetRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
	return new IPFSAsyncImageResponse(ipfsNetRequest, requestedSize, m_cacheDir);
}
