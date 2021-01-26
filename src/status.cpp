
#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QJSEngine>
#include <QVariant>
#include <QStringList>

#include "status.hpp"
#include "libstatus.h"
#include "utils.hpp"
#include "constants.hpp"
#include "QrCode.hpp"


std::map<QString, Status::SignalType> Status::signalMap;
Status *Status::theInstance;

Status *Status::instance()
{
    if (theInstance == 0)
      theInstance = new Status();
    return theInstance;
}


Status::Status(QObject * parent): QObject(parent)
{
  SetSignalEventCallback((void *)&Status::statusGoEventCallback);

  signalMap = {
    {"messages.new", SignalType::Message},
    {"wallet", SignalType::Wallet},
    {"node.ready", SignalType::NodeReady},
    {"node.started", SignalType::NodeStarted},
    {"node.stopped", SignalType::NodeStopped},
    {"node.login", SignalType::NodeLogin},
    {"envelope.sent", SignalType::EnvelopeSent},
    {"envelope.expired", SignalType::EnvelopeExpired},
    {"mailserver.request.completed", SignalType::MailserverRequestCompleted},
    {"mailserver.request.expired", SignalType::MailserverRequestExpired},
    {"discovery.started", SignalType::DiscoveryStarted},
    {"discovery.stopped", SignalType::DiscoveryStopped},
    {"discovery.summary", SignalType::DiscoverySummary},
    {"subscriptions.data", SignalType::SubscriptionsData},
    {"subscriptions.error", SignalType::SubscriptionsError},
    {"whisper.filter.added", SignalType::WhisperFilterAdded}
  };
}


void Status::statusGoEventCallback(const char *event) {
  const QJsonObject signalEvent = QJsonDocument::fromJson(event).object();
  SignalType signalType(Unknown);
  if(!signalMap.count(signalEvent["type"].toString())){
    qWarning() << "Unknown signal: " << signalEvent["type"].toString();
    return;
  }

  signalType = signalMap[signalEvent["type"].toString()];
  
  qDebug() << "Signal received: "<< signalType;
  if(signalType == NodeLogin){
    emit instance()->login(signalEvent["event"]["error"].toString());
  }
}


void Status::closeSession()
{
  QtConcurrent::run([=]{
      // TODO:
      Logout();
      emit logout();
  });
}


QString Status::generateAlias(QString publicKey)
{
  return Utils::generateAlias(publicKey);
}


QString Status::generateIdenticon(QString publicKey)
{
  return Utils::generateIdenticon(publicKey);
}



/*

proc generateQRCodeSVG*(text: string, border: int = 0): string =
  var qr0: array[0..qrcodegen_BUFFER_LEN_MAX, uint8]
  var tempBuffer: array[0..qrcodegen_BUFFER_LEN_MAX, uint8]
  let ok: bool = qrcodegen_encodeText(text, tempBuffer[0].addr, qr0[0].addr, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
  if not ok:
    raise newException(Exception, "Error generating QR Code")
  else:
    var parts: seq[string] = @[]
    let size = qrcodegen_getSize(qr0[0].addr);
    for y in countup(0, size):
      for x in countup(0, size):
        if qrcodegen_getModule(qr0[0].addr, x.cint, y.cint):
          parts.add(&"M{x + border},{y + border}h1v1h-1z")
    let partsStr = parts.join(" ")
    result = &"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"><svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 {size + border * 2} {size + border * 2}\" stroke=\"none\"><rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/><path d=\"{partsStr}\" fill=\"#000000\"/></svg>"

        
        */

QString Status::generateQRCode(QString publicKey)
{
  using namespace qrcodegen;
  // Create the QR Code object
  QStringList svg;
  QrCode qr = QrCode::encodeText( publicKey.toUtf8().data(), QrCode::Ecc::MEDIUM );
  qint32 sz = qr.getSize();
  int border = 2;
  svg << QString("data:image/svg+xml;utf8,");
  svg << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");
  svg << QString("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 %1 %2\" stroke=\"none\"><rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/><path d=\"")
                .arg(sz + border * 2)
                .arg(sz + border * 2);
  for (int y = 0; y < sz; y++)
    for (int x = 0; x < sz; x++)
      if(qr.getModule(x, y))
        svg << QString("M%1,%2h1v1h-1z").arg(x + border).arg(y + border);

  svg << QString("\" fill=\"#000000\"/></svg>");

  return svg.join("");
}
