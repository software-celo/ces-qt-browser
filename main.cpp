#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include "quickwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QtWebEngine::initialize();

    ApplicationEngine appEngine;

    return app.exec();
}
