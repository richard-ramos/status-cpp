#include "status.hpp"
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <QStandardPaths>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QMessageBox>
#include <QObject>
#include <QQmlContext>
#include <QScopedPointer>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "chat-type.hpp"
#include "chat.hpp"
#include "chats-model.hpp"
#include "constants.hpp"
#include "contact.hpp"
#include "contacts-model.hpp"
#include "libstatus.h"
#include "login-model.hpp"
#include "messages-model.hpp"
#include "onboarding-model.hpp"
#include "settings.hpp"
#include "status.hpp"

int main(int argc, char* argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

	QApplication app(argc, argv);

	app.setOrganizationDomain("Status.im");
	app.setApplicationName("Status Desktop");

	QQmlApplicationEngine engine;

	// Only allow a single instance
	QString tmpDir = QDir::tempPath();
	QLockFile lockFile(tmpDir + "/status-desktop.lock");
	if(!lockFile.tryLock(100))
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText("Status Desktop is already running.");
		msgBox.exec();
		return 1;
	}


	auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (path.isEmpty()) qFatal("Cannot determine storage location");
	QDir d{path};
	if (d.mkpath(d.absolutePath()) && QDir::setCurrent(d.absolutePath())) {
		qDebug() << "Created directory in" << QDir::currentPath();  
	}
	



	// Init keystore
	// TODO: extract to separate file
	QString fullDirPath = path; // TODO: set correct path
	const char* initKeystoreResult = InitKeystore(path.toUtf8().data());
	QJsonObject initKeystoreJson = QJsonDocument::fromJson(initKeystoreResult).object();
	if(initKeystoreJson["error"].toString() != "")
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Could not open keystore: " + initKeystoreJson["error"].toString());
		msgBox.exec();
		return 1;
	}
	LoginModel::path = fullDirPath;

	QScopedPointer<Status> status(Status::instance());
	QScopedPointer<Settings> settings(Settings::instance());

	qmlRegisterType<LoginModel>("im.status.desktop", 1, 0, "LoginModel");
	qmlRegisterType<OnboardingModel>("im.status.desktop", 1, 0, "OnboardingModel");
	qmlRegisterType<ChatsModel>("im.status.desktop", 1, 0, "ChatsModel");
	qmlRegisterType<ContactsModel>("im.status.desktop", 1, 0, "ContactsModel");

	qmlRegisterUncreatableType<Chat>("im.status.desktop", 1, 0, "Chat", "Chat class uncreatable");
	qmlRegisterUncreatableType<Contact>("im.status.desktop", 1, 0, "Contact", "Contact class uncreatable");
	qRegisterMetaType<Contact*>("Contact *");

	qRegisterMetaType<Chat*>("Chat *");
	qRegisterMetaType<MessagesModel*>("MessagesModel *");

	qRegisterMetaType<ChatType>("ChatType");
	qmlRegisterUncreatableType<ChatTypeClass>("im.status.desktop", 1, 0, "ChatType", "Not creatable as it is an enum type");

	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "Status", status.get());
	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "StatusSettings", settings.get());

	//engine.load(QUrl(QStringLiteral("../qml/main.qml")));
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}
