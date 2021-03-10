#include <QImage>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QSize>
#include <QNetworkReply>

class IPFSAsyncImageResponse : public QQuickImageResponse
{
	Q_OBJECT
public:
	explicit IPFSAsyncImageResponse(QNetworkRequest req, QSize const& requestedSize, QString const& cacheDir);
	QQuickTextureFactory* textureFactory() const override;

public slots:
	void onResponseFinished();

protected:
	QNetworkAccessManager* m_imageLoader;
	QNetworkReply* m_reply;
	QSize m_requestedSize;
	QImage m_resultImage;
};
