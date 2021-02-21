#include "constants.hpp"
#include <QStandardPaths>
#include <QString>
#include <QFileInfo>


QString Constants::applicationPath(QString path)
{
	return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + path).absoluteFilePath();
}
