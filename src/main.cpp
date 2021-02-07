#include "status.hpp"
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <openssl/ssl.h>
#include <string>

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

#include "chats-model.hpp"
#include "constants.hpp"
#include "libstatus.h"
#include "login-model.hpp"
#include "messages-model.hpp"
#include "onboarding-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "contacts-model.hpp"
#include "contact.hpp"
#include "chat-type.hpp"
#include "chat.hpp"

int main(int argc, char* argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);

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

	// Init keystore
	// TODO: extract to separate file
	QString fullDirPath =
		QCoreApplication::applicationDirPath() + Constants::DataDir; // TODO: set correct path
	const char* initKeystoreResult =
		InitKeystore(QString(fullDirPath + "/keystore").toUtf8().data());
	QJsonObject initKeystoreJson = QJsonDocument::fromJson(initKeystoreResult).object();
	if(initKeystoreJson["error"].toString() != "")
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Could not open keystore: " + initKeystoreJson["error"].toString());
		msgBox.exec();
		return 1;
	}

	QScopedPointer<Status> status(Status::instance());
	QScopedPointer<Settings> settings(Settings::instance());

	qmlRegisterType<LoginModel>("im.status.desktop", 1, 0, "LoginModel");
	qmlRegisterType<OnboardingModel>("im.status.desktop", 1, 0, "OnboardingModel");
	qmlRegisterType<ChatsModel>("im.status.desktop", 1, 0, "ChatsModel");
	qmlRegisterType<ContactsModel>("im.status.desktop", 1, 0, "ContactsModel");

	qmlRegisterUncreatableType<Chat>("im.status.desktop", 1, 0, "Chat", "Chat class uncreatable");
	qmlRegisterUncreatableType<Contact>(
		"im.status.desktop", 1, 0, "Contact", "Contact class uncreatable");
	qRegisterMetaType<Contact*>("Contact *");

	qRegisterMetaType<Chat*>("Chat *");
	qRegisterMetaType<MessagesModel*>("MessagesModel *");

	qRegisterMetaType<ChatType>("ChatType");
	qmlRegisterUncreatableType<ChatTypeClass>(
		"im.status.desktop", 1, 0, "ChatType", "Not creatable as it is an enum type");

	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "Status", status.get());
	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "StatusSettings", settings.get());

	engine.load(QUrl(QStringLiteral("../qml/main.qml")));
	// engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}
