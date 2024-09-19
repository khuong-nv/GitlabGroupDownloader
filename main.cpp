#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include "GitlabRequest.h"
#include "fileopendialog.h"
#include "filesavedialog.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    GitlabRequest gitlab;

    QApplication app(argc, argv);
    qmlRegisterType<FileOpenDialog>("MyModules", 1, 0, "FileOpenDialog");
    qmlRegisterType<FileSaveDialog>("MyModules", 1, 0, "FileSaveDialog");


    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("GitlabRequest", &gitlab);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
