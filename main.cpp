/***************************************************************************************************************
** Author: Peter Fink
** Co-Author: Steffen Kothe
** Description: QtWebEngine based Browser with integration of QT Virtualkeyboard
** Company: Christ Electronic System GmbH
** Last changes: 26.03.2018 -from- Steffen Kothe
**
** Tested with QT Version : 5.9.4  --> Chromium 56 based
**
** Used License: LGPLv3
**
**
**  @Environment variables:
**
**		!Environment variable activates Keyboard!
**
**          ENV --> QT_KEYBOARD_ENABLE
**              If QT_KEYBOARD_ENABLE is undefined, Keyboard is disabled
**              If QT_KEYBOARD_ENABLE=true, Keyboard is enabled
**              If QT_KEYBOARD_ENABLE=false, Keyboard is disabled
**          Only way to active keyboard is to set QT_KEYBOARD_ENABLE=true
**
**      !Environment variable blocks Alert/Confirm Window!
**
**          ENV --> QT_BROWSER_DIALOGS_BLOCK
**              If QT_BROWSER_DIALOGS_BLOCK is undefined, Alert/Dialogs are blocked
**              If QT_BROWSER_DIALOGS_BLOCK = false, Alert/Dialogs are not blocked
**              If QT_BROWSER_DIALOGS_BLOCK = true , Alert/Dialogs are blocked
**          Only way to activate Alerts/Dialogs is to set QT_BROWSER_DIALOGS_BLOCK = false
**
**
**  @Description/Changes:
* 		Disabled Cache/History storing on disk. Cache/History is saved/hold in RAM
** 		Disabled Chromium standard error page. Use of custom error page
**		Integration of customizable error landing page (/usr/share/minimalbrowser/erl/error.html
**  	Integration of QT Virtualkeyboard.
**      Handle context menu request --> "disable" context menu
**      Possibility to disable/enable Alerts/Dialogs called from HTML/Javascript ( BUGS )
**
**
**  @Known Bugs:
**       1       Alert/Dialog Window in QTWebengine does not work correctly
**               Possible solution --> Change DefaultWindowDecorate.qml to working qml code
**
**       2       Alert/Dialog Window Requested provokates following message:
**               transient parent QQuickWindowQmlImpl_QML_19(0x34fb68) can not be same as window"
**
**
**       3       QtWebEngine Framework does not compile correct with thumb instruction set,
**               use arm instruction set instead
**
**       4       Yocto possible does not include "qtquickcontrols2" and "qtquickcontrols2-qmlplugins"
**               to build. QtWebEngine browser depends on this modules. Include them to build.
**               A not including of this packages result in a faulty behavior with alert/confirm dialogs.
**               Disable Alert/Block Dialogs prevent faulty behavior in this case.
**
**       5       In some cases, emulated context shift with symbolinsertion does not work.
**                      Works currently not in:
**                              -   Angular Dialogwindows
**
***************************************************************************************************************/


#include <QQuickView>
#include <QQmlEngine>
#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include <QtWebEngine/qquickwebengineprofile.h>
#include "quickwindow.h"
#include <QProcessEnvironment>
#include <QTextStream>
#include <QtGlobal>

int main(int argc, char *argv[]){
	QProcessEnvironment env;
	bool keyboard_env_exist;
	bool keyboard_enable;
	bool block_dialog;
	QString envValue;


	/*Proof for load of QT Virtual Keyboard*/
	env=QProcessEnvironment::systemEnvironment();
	keyboard_env_exist = env.contains(QString("QT_KEYBOARD_ENABLE"));
	if( keyboard_env_exist == true){
		envValue=env.value(QString("QT_KEYBOARD_ENABLE"),nullptr);
		qInfo("Keyboard enable is: %s", envValue.toStdString().c_str());
		if(envValue.compare(QString("true")) == 0){
			keyboard_enable = true;
			qInfo("Enable Qt Keyboardplugin");
			qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
		}else{
			keyboard_enable  = false;
			qInfo("Disable Qt Keyboardplugin");
		}
	}else{
		keyboard_enable = false;
		qInfo("Disable Qt Keyboardplugin");
	}


    /*Environment Variable for blocking Alert/Confirm Dialogs in Browser*/
	if( true == env.contains(QString("QT_BROWSER_DIALOGS_BLOCK"))){
		envValue=env.value(QString("QT_BROWSER_DIALOGS_BLOCK"),nullptr);

		if(envValue.compare(QString("false")) == 0){
			block_dialog  = false;
		}else{
			block_dialog   = true;
		}

	}else{
		block_dialog = true;
	}


	QGuiApplication app(argc, argv);
	QtWebEngine::initialize();
	ApplicationEngine appEngine(keyboard_enable,block_dialog);


	return app.exec();
}
