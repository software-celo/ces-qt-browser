#include "idlehelper.h"

IdleHelper::IdleHelper( QObject* parent )
      : QObject( parent )
{
    QTimer::singleShot(0, this, &IdleHelper::start);
}

void IdleHelper::start()
{
    m_idleThread = new QThread();
    m_notifier = new InputNotifier();
    m_notifier->moveToThread(m_idleThread);
    // connections between idleThread and notifier
    QObject::connect(m_idleThread, &QThread::started, m_notifier, &InputNotifier::start);
    QObject::connect(m_notifier, &InputNotifier::finished, m_idleThread, &QThread::quit);
    QObject::connect(m_notifier, &InputNotifier::finished, m_idleThread, &QThread::deleteLater);
    QObject::connect(m_idleThread, &QThread::finished, m_notifier, &InputNotifier::deleteLater);

    // connections between notifier and this (idleHelper)
    QObject::connect(m_notifier, &InputNotifier::lockTimeout, this, &IdleHelper::lockTimeout);
    QObject::connect(m_notifier, &InputNotifier::blankTimeout, this, &IdleHelper::blankTimeout);
    QObject::connect(m_notifier, &InputNotifier::unblank, this, &IdleHelper::unblank);

    m_idleThread->start();
}

void IdleHelper::unlock()
{
    m_notifier->unlock();

//    why is sending a signal not working here?! TODO!
//    emit unlockSignal();
//    with this in start: //    QObject::connect(this, &IdleHelper::unlockSignal, m_notifier, &InputNotifier::unlock);
//    alternatively: //QMetaObject::invokeMethod(m_notifier, "unlock", Qt::BlockingQueuedConnection);
}


