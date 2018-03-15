#include <QtGui/QGuiApplication>
//#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include "quickwindow.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QtWebEngine::initialize();

//    IdleHelper *globalIdleHelper = new IdleHelper(0);

//    app.installEventFilter(globalIdleHelper);

    ApplicationEngine appEngine;
//    ApplicationEngine *appEngine = new ApplicationEngine(globalIdleHelper);

//    appEngine.installEventFilter(globalIdleHelper);

    return app.exec();
}
