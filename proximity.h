#ifndef PROXIMITY_H
#define PROXIMITY_H

#define QT_NO_DEBUG_OUTPUT 1

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QSocketNotifier>
#include <unistd.h>

class Proximity : public QObject
{    Q_OBJECT
public:
    explicit Proximity(QObject* parent);
    ~Proximity();

    void setup(int pwm, int pwmChip, int gpio, int period, int dutyCycle);

public slots:
    void enable();
    void disable();
    void startPollMode();

signals:
    void objectDetected();

private:
    QString m_irqGPIO;
    QString m_pwm;
    QString m_pwmChip;

    QString m_gpioPath;
    QString m_pwmPath;
    QString m_pwmDutyCycle;
    QString m_pwmPeriod;

    bool m_enabled;
    bool m_statusOK;

    QFile* m_irqFile;
    QSocketNotifier* m_irq;
    QTimer m_checkTimer;

    void checkForObject();

    void enablePWM();
    void disablePWM();
};


#endif // PROXIMITY_H

