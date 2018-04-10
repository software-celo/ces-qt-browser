#include "configBackend.h"

ConfigBackend::ConfigBackend(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings(CESFILE, QSettings::IniFormat);

    QTimer::singleShot(0, this, SLOT(readCESConfig()));
}


ConfigBackend::~ConfigBackend()
{
    delete m_settings;
}


bool ConfigBackend::getProximityEnable()
{
    return m_proximityEnable;
}


int ConfigBackend::getLockTime()
{
    return m_lockTime;
}


int ConfigBackend::getBlankTime()
{
    return m_blankTime;
}


bool ConfigBackend::getBlankEnable()
{
    return m_blankEnable;
}


bool ConfigBackend::getLockEnable()
{
    return m_lockEnable;
}


int ConfigBackend::getResetTime()
{
    return m_resetTime;
}


bool ConfigBackend::getResetEnable()
{
    return m_resetEnable;
}


int ConfigBackend::getLockBrightness()
{
    return m_lockBrightness;
}


int ConfigBackend::getBlankBrightness()
{
    return m_blankBrightness;
}


int ConfigBackend::getUnlockBrightness()
{
    return m_unlockBrightness;
}


int ConfigBackend::getProximityGPIO()
{
    return m_proximityGPIO;
}


int ConfigBackend::getProximityPWM()
{
    return m_proximityPWM;
}


int ConfigBackend::getProximityPWMChip()
{
    return m_proximityPWMChip;
}


int ConfigBackend::getPWMDutyCycle()
{
    return m_pwmDutyCycle;
}


int ConfigBackend::getPWMPeriod()
{
    return m_pwmPeriod;
}


void ConfigBackend::readCESConfig()
{
    m_lockTime = getIntFromSettings("lockTime", "Screensaver", 3600);
    m_blankTime = getIntFromSettings("blankTime", "Screensaver", 3600);
    m_resetTime = getIntFromSettings("resetTime", "Reset", 3);

    m_blankBrightness = getIntFromSettings("blankBrightness", "Screensaver", 0);
    m_lockBrightness = getIntFromSettings("lockBrightness", "Screensaver", 4);
    m_unlockBrightness = getIntFromSettings("unlockBrightness", "Screensaver", 7);

    m_lockEnable = getBoolFromSettings("lockEnable", "Screensaver", false);
    m_blankEnable = getBoolFromSettings("blankEnable", "Screensaver", true);
    m_resetEnable = getBoolFromSettings("resetEnable", "Reset", false);
    m_proximityEnable = getBoolFromSettings("proximityEnable", "Proximity", false);

    m_proximityGPIO = getIntFromSettings("proximityGPIO", "Proximity", 89);
    m_proximityPWMChip = getIntFromSettings("proximityPWMChip", "Proximity", 1);
    m_proximityPWM = getIntFromSettings("proximityPWM", "Proximity", 4);

    m_pwmDutyCycle = getIntFromSettings("pwmDutyCycle", "Proximity", 4000);
    m_pwmPeriod = getIntFromSettings("pwmPeriod", "Proximity", 27000);

    return;
}

bool ConfigBackend::getBoolFromSettings(QString key, QString group, bool preset=false)
{
    bool value;
    m_settings->beginGroup(group);

    if ( m_settings->contains(key) ){
        value = m_settings->value(key).toBool();
    }
    else
        value = preset;

    m_settings->endGroup();

    return value;
}


int ConfigBackend::getIntFromSettings(QString key, QString group, int preset=0)
{
    int value;
    m_settings->beginGroup(group);

    if ( m_settings->contains(key) ){
        value = m_settings->value(key).toInt();
    }
    else
        value = preset;

    if ( value < 0 )
        value = 0;

    m_settings->endGroup();

    return value;
}

void ConfigBackend::rebootSystem()
{
    m_settings->sync();
    sync();
    if ( -1 == reboot(LINUX_REBOOT_CMD_RESTART ))
        QCoreApplication::quit();
}
