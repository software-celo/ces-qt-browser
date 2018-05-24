#include "idlehelper.h"

IdleHelper::IdleHelper(QObject* parent , ConfigBackend *cfgBackend)
      : QObject( parent )
{
    m_cfgBackend = cfgBackend;
    m_dailyReset = false;
    m_resetToday = true;
    m_resetTimer.setInterval(900000);      // check every 15 minutes
    m_resetTime = 3;                        // resulting in a reset signal between 3:00AM and 3:15AM
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &IdleHelper::checkDailyReset);
    m_resetTimer.start();

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

    QObject::connect(this, &IdleHelper::unlockNotifier, m_notifier, &InputNotifier::unlock);

    m_proximity = new Proximity(this);

    m_idleThread->start();

    // connections between notifier/idelHelper and proximity
    QObject::connect(m_proximity, &Proximity::objectDetected, m_notifier, &InputNotifier::inputEvent);

    QObject::connect(m_proximity, &Proximity::objectDetected, this, &IdleHelper::inputEvent);

    QTimer::singleShot(0, this, &IdleHelper::reloadConfig);
}

void IdleHelper::unlock()
{
    emit unlockNotifier();
}

bool IdleHelper::getBlankEnable()
{
    return m_cfgBackend->getBlankEnable();
}

bool IdleHelper::getLockEnable()
{
    return m_cfgBackend->getLockEnable();
}

void IdleHelper::reloadConfig()
{
    m_notifier->setLockEnable(m_cfgBackend->getLockEnable());
    m_notifier->setBlankEnable(m_cfgBackend->getBlankEnable());
    m_notifier->setTimeoutLock(m_cfgBackend->getLockTime());
    m_notifier->setTimeoutBlank(m_cfgBackend->getBlankTime());
    m_resetTime = m_cfgBackend->getResetTime();
    m_dailyReset = m_cfgBackend->getResetEnable();
    m_proximityEnable = m_cfgBackend->getProximityEnable();
    m_screensaverEnable = m_cfgBackend->getScreensaverEnable();
    if ( m_proximityEnable ) {
        m_proximity->setup(m_cfgBackend->getProximityPWM(), m_cfgBackend->getProximityPWMChip(), \
                       m_cfgBackend->getProximityGPIO(), m_cfgBackend->getPWMPeriod(),m_cfgBackend->getPWMDutyCycle());
    }
    if ( m_screensaverEnable == false ) {
        m_notifier->stop();
        emit inputEvent();
    }

    emit lockEnableChanged();
    emit blankEnableChanged();
    return;
}

void IdleHelper::checkDailyReset()
{
    QTime now = QTime::currentTime();
    if (now.hour() == m_resetTime){
        if((m_dailyReset == true) && (m_resetToday == false)){
            emit dailyReset();
            m_resetToday = true;
        }
    }
    else {
        m_resetToday = false;
    }
}

IdleHelper::~IdleHelper()
{
    if (m_proximity)
        delete m_proximity;
}
