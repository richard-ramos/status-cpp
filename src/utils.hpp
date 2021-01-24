#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
public:
    static QString generateAlias(QString publicKey);
    static QString generateIdenticon(QString publicKey);


    static QString jsonToStr(QJsonObject & obj);
    static QString jsonToStr(QJsonArray & arr);
};

#endif // UTILS_H