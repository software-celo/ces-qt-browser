/***************************************************************************************************************
** Copyright (c) 2015-2022 Christ Electronic Systems GmbH
** Copyright (c) 2015-2018 Peter Fink <pfink@christ-es.de>
** Copyright (c) 2017-2018 Steffen Kothe <skothe@christ-es.de>
**
** Description: QtWebEngine based Browser with integration of QT Virtualkeyboard and Rotation
**
** Tested with QT Version : 5.9.4  --> Chromium 56 based
**
** Used License: LGPLv3
**
**
**  @Description/Changes:
**               Added c++ code for enable/disbale keyboard, dialogs
**               Added c++ code for 0,90,180,270 degree rotation of window
**               Added activated text selection function for window (text not selectable)
**               Added fixed qml/javascript function, performing context shift
**               Rework code (fixed bugs, removed obsolote files, removed environment settings for keyboard and dialogs)rd
**               Qt Keyboard does not show lists anymore ( language key is toggeling all languages)
**
**  @Known Bugs:
**       1       Dialog windows in QTWebengine does not work correctly
**                  Possible solution --> Change DefaultWindowDecorate.qml to working qml code
**                  QT integrated Dialogs does not work with rotation of webengineview!
**                  QTVirtualKeyboard is not optimized for use with dialogs (dialogs overlay keyboard)
**                  Solution: Add own customized dialogs for: File,Color,Javascript, Authentication
**
**       2       Alert/Dialog window requests provokates following message:
**               transient parent QQuickWindowQmlImpl_QML_19(0x34fb68) can not be same as window"
**
**       3       QtWebEngine Framework does not compile correct with thumb instruction set,
**               use arm instruction set instead
**
**       4       Yocto possible does not include "qtquickcontrols2" and "qtquickcontrols2-qmlplugins"
**               to build. QtWebEngine browser depends on this modules. Include them to build.
**               A not including of this packages result in a faulty behavior with alert/confirm dialogs.
**               Disable Alert/Block Dialogs prevent faulty behavior in this case.
**
**
***************************************************************************************************************/

#include <QQuickView>
#include <QQmlEngine>
#include <QtGui/QGuiApplication>
#include <QtWebEngine/qtwebengineglobal.h>
#include <QtWebEngine/qquickwebengineprofile.h>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QtGlobal>
#include "quickwindow.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QtWebEngine::initialize();
    QGuiApplication app(argc, argv);
    ApplicationEngine appEngine;


    /*  Disables cursor/mouse symbol in application */
    //  app.setOverrideCursor(QCursor(Qt::BlankCursor));


    return app.exec();
}
