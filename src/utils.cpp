#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "libstatus.h"
#include "utils.hpp"

QString Utils::generateAlias(QString publicKey)
{
  return QString(GenerateAlias(publicKey.toUtf8().data()));
}

QString Utils::generateIdenticon(QString publicKey)
{
  return QString(Identicon(publicKey.toUtf8().data()));
}

QString Utils::jsonToStr(QJsonObject & obj) {
  QJsonDocument doc(obj);
  return QString::fromUtf8(doc.toJson());
}

QString Utils::jsonToStr(QJsonArray & arr) {
  QJsonDocument doc(arr);
  return QString::fromUtf8(doc.toJson());
}
