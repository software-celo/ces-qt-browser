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
**
** Edited by:
**
** Author: Peter Fink
** Author: Steffen Kothe
** Description: QtWebEngine based Browser with integration of Qt Virtualkeyboard
** Company: Christ Electronic System GmbH
** Used License: LGPLv3
** Copyright (C) 2017-2022 Christ Electronic Systems GmbH
**
** For more details: main.cpp
**
****************************************************************************/

#include "quickwindow.h"


ApplicationEngine::ApplicationEngine()
{
    m_configBackend = new ConfigBackend(this);
    m_idleHelper = new IdleHelper(this, m_configBackend);
    m_backlight = new Backlight(this);

    QObject::connect(m_configBackend, &ConfigBackend::configChanged, this, &ApplicationEngine::configBacklight);

    /*Config the backlight anyway here as the configBackend might already be ready*/
    m_backlight->setBlankBrightness(m_configBackend->getBlankBrightness());
    m_backlight->setLockBrightness(m_configBackend->getLockBrightness());
    m_backlight->setUnlockBrightness(m_configBackend->getUnlockBrightness());

    rootContext()->setContextProperty("_backlight", m_backlight);
    rootContext()->setContextProperty("_idleHelper", m_idleHelper);
    rootContext()->setContextProperty("_configBackend", m_configBackend);

    load(QUrl("qrc:/main.qml"));

    /*Calls a method, which loads a url to webengine*/
    QMetaObject::invokeMethod(rootObjects().constFirst(), "load", Q_ARG(QVariant, startupUrl()));
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


/* Return a QUrl object and check if it's a file
 *
 * Always return a QUrl Object depending on userinput (path or url)
*/
QUrl ApplicationEngine::urlFromUserInput(const QString& userInput)
{
    QUrl url = QUrl(userInput);
    //  bool exists = QFileInfo(QUrl(userInput).path()).exists();

    return url;
}


/* Try to find possible Start URLs in passed arguments to application
 *
 * Check for non option flags and try to find an valid url
 *
 * In case of success, valid url is returned, else error landing page path is
 * returned.
*/
QUrl ApplicationEngine::startupUrl()
{
    QUrl ret;
    QStringList args(qApp->arguments());
    args.takeFirst();
    Q_FOREACH (const QString& arg, args) {
        if (arg.startsWith(QLatin1Char('-')))
             continue;
        ret = this->urlFromUserInput(arg);
        if (ret.isValid())
            return ret;
    }
    return QUrl(QStringLiteral("file:///usr/share/ces-qt-browser/erl/error.html"));
}
