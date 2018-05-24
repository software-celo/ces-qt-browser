/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "quickwindow.h"
#include "util.h"

class Utils : public QObject {
    Q_OBJECT
public:
    Utils(QObject* parent = 0) : QObject(parent) { }
    Q_INVOKABLE static QUrl fromUserInput(const QString& userInput) { return urlFromUserInput(userInput); }
};

#include "quickwindow.moc"

ApplicationEngine::ApplicationEngine(bool keyboard, bool dialog_block)
{
    m_configBackend = new ConfigBackend(this);
    m_idleHelper = new IdleHelper(this, m_configBackend);

    rootContext()->setContextProperty("_utils", new Utils(this));

    m_backlight = new Backlight(this);

    QObject::connect(m_configBackend, &ConfigBackend::configReady, this, &ApplicationEngine::configBacklight);

    /*Config the backlight anyway here as the configBackend might already be ready*/
    m_backlight->setBlankBrightness(m_configBackend->getBlankBrightness());
    m_backlight->setLockBrightness(m_configBackend->getLockBrightness());
    m_backlight->setUnlockBrightness(m_configBackend->getUnlockBrightness());

    rootContext()->setContextProperty("_backlight", m_backlight);
    rootContext()->setContextProperty("_idleHelper", m_idleHelper);

    if(keyboard ==  true){
        qInfo("Keyboard enabled");
		load(QUrl("qrc:/main.qml"));
	}else{
        qInfo("Keyboard disabled");
		load(QUrl("qrc:/main_nok.qml"));
	}

    /*Calls a method, which handles the dialog request*/
    QMetaObject::invokeMethod(rootObjects().first(), "blockDialogs", Q_ARG(QVariant, dialog_block));

    /*Calls a method, which loads a url to webengine*/
    QMetaObject::invokeMethod(rootObjects().first(), "load", Q_ARG(QVariant, startupUrl()));
}

void ApplicationEngine::configBacklight()
{
    m_backlight->setBlankBrightness(m_configBackend->getBlankBrightness());
    m_backlight->setLockBrightness(m_configBackend->getLockBrightness());
    m_backlight->setUnlockBrightness(m_configBackend->getUnlockBrightness());
}

ApplicationEngine::~ApplicationEngine()
{
    delete m_idleHelper;
    delete m_backlight;
    delete m_configBackend;
}
