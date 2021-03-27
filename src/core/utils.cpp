#include "utils.hpp"
#include "QrCode.hpp"
#include "base58.h"
#include "libstatus.h"
#include "uint256_t.h"
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QString>
#include <QTextDocumentFragment>
#include <QVector>

Utils::Utils(QObject* parent)
	: QObject(parent)
{ }

QString Utils::generateAlias(QString publicKey)
{
	if(publicKey.isEmpty()) return "";
	return QString(GenerateAlias(publicKey.toUtf8().data()));
}

QString Utils::generateIdenticon(QString publicKey)
{
	if(publicKey.isEmpty()) return "";
	return QString(Identicon(publicKey.toUtf8().data()));
}

QString Utils::jsonToStr(QJsonObject obj)
{
	QJsonDocument doc(obj);
	return QString::fromUtf8(doc.toJson());
}

QString Utils::jsonToStr(QJsonArray arr)
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
			if(qr.getModule(x, y)) svg << QString("M%1,%2h1v1h-1z").arg(x + border).arg(y + border);

	svg << QString("\" fill=\"#000000\"/></svg>");

	return svg.join("");
}

QString Utils::plainText(const QString& value)
{
	return QTextDocumentFragment::fromHtml(value).toPlainText();
}

QString Utils::decodeHash(QString ednHash)
{

	if(ednHash.left(2) != QStringLiteral("e3"))
	{
		qWarning() << "Could not decode sticker. It may still be valid, but requires a different codec to be used: " + ednHash;
		return "";
	}

	if(ednHash.left(8) == QStringLiteral("e3010170"))
	{
		ednHash.remove(0, 8);
	}
	else if(ednHash.left(6) == QStringLiteral("e30170"))
	{
		ednHash.remove(0, 6);
	}

	std::vector<unsigned char> vch;
	for(int i = 0; i < ednHash.length(); i += 2)
	{
		QString byteString = ednHash.mid(i, 2);
		unsigned char b = std::strtol(byteString.toUtf8().data(), NULL, 16);
		vch.push_back(b);
	}

	return QString::fromStdString(EncodeBase58(vch));
}

// Optimized recursive solution to calculate `pow(x, n)`
// using divide-and-conquer
uint256_t power(uint256_t x, uint256_t n)
{
	// base condition
	if(n == uint256_t(0))
	{
		return uint256_t(1);
	}

	// calculate subproblem recursively
	uint256_t pow = power(x, n / uint256_t(2));

	if(n & uint256_t(1))
	{ // if `y` is odd
		return x * pow * pow;
	}

	// otherwise, `y` is even
	return pow * pow;
}

QString Utils::wei2Token(QString input, int decimals)
{
	uint256_t one_eth = power(uint256_t(10), uint256_t(decimals));
	uint256_t inp = uint256_t(input.toStdString(), 10);
	uint256_t eth = inp / one_eth;
	uint256_t remainder = inp % one_eth;

	QString decimalPart = QString::fromStdString(remainder.str(10, decimals));
	decimalPart.remove(QRegExp("0*$"));
	if(decimalPart != "")
	{
		decimalPart = "." + decimalPart;
	}

	return QString::fromStdString(eth.str()) + decimalPart;
}