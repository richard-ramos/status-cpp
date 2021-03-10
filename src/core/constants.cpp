#include "constants.hpp"
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

QString Constants::applicationPath(QString path)
{
	return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + path).absoluteFilePath();
}

QString Constants::tmpPath(QString path)
{
	return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + path).absoluteFilePath();
}

QString Constants::cachePath(QString path)
{
	return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + path).absoluteFilePath();
}

QString Constants::getTimelineChatId(QString pubKey)
{
	if(pubKey.isEmpty())
	{
		return "@timeline70bd746ddcc12beb96b2c9d572d0784ab137ffc774f5383e50585a932080b57cca0484b259e61cecbaa33a4c98a300a";
	}
	else
	{
		return "@" + pubKey;
	}
}
