#ifndef CONFIGBACKEND_H
#define CONFIGBACKEND_H

#define QT_NO_DEBUG_OUTPUT 1

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#define CESFILE "/etc/ces.conf"

class ConfigBackend : public QObject
{
    Q_OBJECT
public:
    explicit ConfigBackend(QObject *parent = 0);
    ~ConfigBackend();

    bool getProximityEnable();
    bool getScreensaverEnable();
    int getProximityGPIO();
    int getProximityPWM();
    int getProximityPWMChip();
    int getPWMPeriod();
    int getPWMDutyCycle();

    int getLockTime();
    int getBlankTime();
    bool getBlankEnable();
    bool getLockEnable();

    int getResetTime();
    bool getResetEnable();

    int getLockBrightness();
    int getBlankBrightness();
    int getUnlockBrightness();

public slots:
    void rebootSystem();
    void readCESConfig();

signals:
    void configReady();

private:
    bool m_proximityEnable;
    bool m_screensaverEnable;
    bool m_blankEnable;
    bool m_lockEnable;
    bool m_resetEnable;

    int m_proximityPWM;
    int m_proximityPWMChip;
    int m_proximityGPIO;
    int m_pwmDutyCycle;
    int m_pwmPeriod;
    int m_lockTime;
    int m_blankTime;
    int m_resetTime;
    int m_unlockBrightness;
    int m_lockBrightness;
    int m_blankBrightness;

    QSettings* m_settings;

    int getIntFromSettings(QString key, QString group, int preset);
    bool getBoolFromSettings(QString key, QString group, bool preset);
};

#endif // CONFIGBACKEND_H
