#include <QDateTime>
#include <QDebug>
#include <QString>

void logFormatter(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	const char* file = context.file ? context.file : "";
	const char* function =
		context.function ? (QString(QStringLiteral("\033[0;33mfunction=\033[94m") + QString(context.function)).toLocal8Bit().constData()) : "";
	QByteArray timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toLocal8Bit();

	const char* log;

	switch(type)
	{
	case QtDebugMsg: log = "\033[0;90mDBG \033[0m%s \033[1m%s \033[0;33mfile=\033[94m%s:%u %s\n"; break;
	case QtInfoMsg: log = "\033[0;36mINF \033[0m%s \033[1m%s \033[0;33mfile=\033[94m%s:%u %s\n"; break;
	case QtWarningMsg: log = "\033[0;33mWRN \033[0m%s \033[1m%s \033[0;33mfile=\033[94m%s:%u %s\n"; break;
	case QtCriticalMsg: log = "\033[0;91mCRT \033[0m%s \033[1m%s \033[0;33mfile=\033[94m%s:%u %s\n"; break;
	case QtFatalMsg: log = "\031[0;31mFAT \033[0m%s \033[1m%s \033[0;33mfile=\033[94m%s:%u %s\n"; break;
	}
	fprintf(stderr, log, timestamp.constData(), localMsg.constData(), file, context.line, function);
}