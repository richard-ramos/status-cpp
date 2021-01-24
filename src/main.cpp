#include <openssl/ssl.h>
#include "status.hpp"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>

#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QApplication>
#include <QObject>
#include <QScopedPointer>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "onboarding-model.hpp"
#include "status.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;


    // Only allow a single instance
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/status-desktop.lock");
    if(!lockFile.tryLock(100)){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Status Desktop is already running.");
        msgBox.exec();
        return 1;
    }


    QScopedPointer<Status> status(Status::instance());


    qmlRegisterType<OnboardingModel>("im.status.desktop", 1, 0, "OnboardingModel");
    qmlRegisterSingletonInstance("im.status.desktop", 1, 0, "Status", status.get());


    engine.load(QUrl(QStringLiteral("../qml/main.qml")));
    // engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    app.exec();
}


