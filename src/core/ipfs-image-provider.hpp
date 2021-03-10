#include <QNetworkRequest>
#include <QQuickAsyncImageProvider>
#include <QString>

class IPFSAsyncImageProvider : public QQuickAsyncImageProvider
{
public:
	explicit IPFSAsyncImageProvider(QString ipfsTmpDir, QString ipfsGateway);
	QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
	QString m_gateway;
	QString m_cacheDir;
};
