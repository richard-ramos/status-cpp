#include "constants.hpp"
#include <QStandardPaths>
#include <QString>


QString Constants::applicationPath(QString path)
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + path;
}
