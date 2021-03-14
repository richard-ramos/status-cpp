#include "chat-type.hpp"
#include "chat.hpp"
#include "chats-model.hpp"
#include "constants.hpp"
#include "contact.hpp"
#include "contacts-model.hpp"
#include "content-type.hpp"
#include "custom-networks-model.hpp"
#include "devices-model.hpp"
#include "ens-model.hpp"
#include "ens-utils.hpp"
#include "libstatus.h"
#include "login-model.hpp"
#include "logs.hpp"
#include "mailserver-model.hpp"
#include "mailserver-cycle.hpp"
#include "messages-model.hpp"
#include "onboarding-model.hpp"
#include "settings.hpp"
#include "status.hpp"
#include "stickerpack.hpp"
#include "stickers-model.hpp"
#include "utils.hpp"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QMessageBox>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QResource>
#include <QScopedPointer>
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <openssl/ssl.h>
#include <string>
#include "ipfs-image-provider.hpp"

int main(int argc, char* argv[])
{
	qInstallMessageHandler(logFormatter);

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

	QApplication app(argc, argv);
	app.setOrganizationName("Status");
	app.setOrganizationDomain("status.im");
	app.setApplicationName("Status Desktop");

#ifndef NDEBUG
	app.setWindowIcon(QIcon(":/resources/status-dev.svg"));
#else
	app.setWindowIcon(QIcon(":/resources/status.svg"));
#endif

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

	// TODO: refactor

	// Initialize app directory
	if(Constants::applicationPath().isEmpty())
	{
		QDir d{Constants::applicationPath()};
		if(!d.mkpath(d.absolutePath()))
		{
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setText("Cannot determine storage location");
			msgBox.exec();
			return 1;
		}
	}

	// Initialize cache directories
	if(Constants::cachePath("/stickers/network").isEmpty())
	{
		QDir d{Constants::cachePath("/stickers/network")};
		if(!d.mkpath(d.absolutePath()))
		{
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setText("Cannot determine storage location");
			msgBox.exec();
			return 1;
		}
	}

	// Initialize cache directories
	if(Constants::cachePath("/ipfs").isEmpty())
	{
		QDir d{Constants::cachePath("/ipfs")};
		if(!d.mkpath(d.absolutePath()))
		{
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setText("Cannot determine storage location");
			msgBox.exec();
			return 1;
		}
	}


	IPFSAsyncImageProvider* imageProvider = new IPFSAsyncImageProvider(Constants::cachePath("/ipfs"), Constants::StatusIPFS);
	engine.addImageProvider("ipfs-cache", imageProvider);

	// Init keystore
	const char* initKeystoreResult = InitKeystore(Constants::applicationPath(Constants::Keystore).toUtf8().data());
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
	QScopedPointer<Ens::Utils> ensUtils(new Ens::Utils());

	qmlRegisterType<LoginModel>("im.status.desktop", 1, 0, "LoginModel");
	qmlRegisterType<OnboardingModel>("im.status.desktop", 1, 0, "OnboardingModel");
	qmlRegisterType<ChatsModel>("im.status.desktop", 1, 0, "ChatsModel");
	qmlRegisterType<ContactsModel>("im.status.desktop", 1, 0, "ContactsModel");
	qmlRegisterType<CustomNetworksModel>("im.status.desktop", 1, 0, "CustomNetworksModel");
	qmlRegisterType<ENSModel>("im.status.desktop", 1, 0, "ENSModel");
	qmlRegisterType<DevicesModel>("im.status.desktop", 1, 0, "DevicesModel");
	qmlRegisterType<StickerPacksModel>("im.status.desktop", 1, 0, "StickerPacksModel");
	qmlRegisterType<MailserverModel>("im.status.desktop", 1, 0, "MailserverModel");

	qmlRegisterUncreatableType<Chat>("im.status.desktop", 1, 0, "Chat", "Chat class uncreatable");
	qmlRegisterUncreatableType<Contact>("im.status.desktop", 1, 0, "Contact", "Contact class uncreatable");
	qmlRegisterUncreatableType<StickerPack>("im.status.desktop", 1, 0, "StickerPack", "StickerPack class uncreatable");

	qRegisterMetaType<Contact*>("Contact *");
	qRegisterMetaType<Message*>("Message *");
	qRegisterMetaType<Chat*>("Chat *");
	qRegisterMetaType<MessagesModel*>("MessagesModel *");
	qRegisterMetaType<StickerPack*>("StickerPack *");
	qRegisterMetaType<MailserverCycle*>("MailserverCycle *");

	qRegisterMetaType<Device>("Device");
	qRegisterMetaType<Mailserver>("Mailserver");
	qRegisterMetaType<CustomNetwork>("CustomNetwork");
	qRegisterMetaType<Topic>("Topic");
	qRegisterMetaType<ChatMember>("ChatMember");

	qRegisterMetaType<ChatType>("ChatType");
	qmlRegisterUncreatableType<ChatTypeClass>("im.status.desktop", 1, 0, "ChatType", "Not creatable as it is an enum type");
	qRegisterMetaType<ContentType>("ContentType");
	qmlRegisterUncreatableType<ContentTypeClass>("im.status.desktop", 1, 0, "ContentType", "Not creatable as it is an enum type");

	qmlRegisterSingletonType<Utils>("im.status.desktop", 1, 0, "StatusUtils", utilsProvider);

	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "Status", status.get());
	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "StatusSettings", settings.get());
	qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "EnsUtils", ensUtils.get());

	// TODO: check windows path
	QResource::registerResource("./static-resources.rcc");

	QObject::connect(Settings::instance(), &Settings::localeChanged, &engine, &QQmlApplicationEngine::retranslate);

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}
