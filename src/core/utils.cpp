#include "utils.hpp"
#include "QrCode.hpp"
#include "libstatus.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTextDocumentFragment>
#include <QVector>

Utils::Utils(QObject* parent)
	: QObject(parent)
{ }

QString Utils::generateAlias(QString publicKey)
{
	if(publicKey.isEmpty())
		return "";
	return QString(GenerateAlias(publicKey.toUtf8().data()));
}

QString Utils::generateIdenticon(QString publicKey)
{
	if(publicKey.isEmpty())
		return "";
	return QString(Identicon(publicKey.toUtf8().data()));
}

QString Utils::jsonToStr(QJsonObject& obj)
{
	QJsonDocument doc(obj);
	return QString::fromUtf8(doc.toJson());
}

QString Utils::jsonToStr(QJsonArray& arr)
{
	QJsonDocument doc(arr);
	return QString::fromUtf8(doc.toJson());
}

QJsonArray Utils::toJsonArray(const QVector<QString>& value)
{
	QJsonArray array;
	for(auto& v : value)
		array << v;
	return array;
}

void Utils::copyToClipboard(const QString& value)
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	clipboard->setText(value);
}

QVector<QString> Utils::toStringVector(const QJsonArray& arr)
{
	QVector<QString> result;
	foreach(const QJsonValue& value, arr)
	{
		result << value.toString();
	}
	return result;
}

QString Utils::generateQRCode(QString publicKey)
{
	using namespace qrcodegen;
	// Create the QR Code object
	QStringList svg;
	QrCode qr = QrCode::encodeText(publicKey.toUtf8().data(), QrCode::Ecc::MEDIUM);
	qint32 sz = qr.getSize();
	int border = 2;
	svg << QString("data:image/svg+xml;utf8,");
	svg << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE svg PUBLIC \"-//W3C//DTD "
				   "SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");
	svg << QString("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %1 %2\" "
				   "stroke=\"none\"><rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/><path d=\"")
			   .arg(sz + border * 2)
			   .arg(sz + border * 2);
	for(int y = 0; y < sz; y++)
		for(int x = 0; x < sz; x++)
			if(qr.getModule(x, y))
				svg << QString("M%1,%2h1v1h-1z").arg(x + border).arg(y + border);

	svg << QString("\" fill=\"#000000\"/></svg>");

	return svg.join("");
}

QString Utils::plainText(const QString& value)
{
	return QTextDocumentFragment::fromHtml(value).toPlainText();
}
