#include "proximity.h"

Proximity::Proximity(QObject* parent = 0): QObject(parent)
{
    m_gpioPath = "/sys/class/gpio/";
    m_pwmPath = "/sys/class/pwm/";
    m_statusOK = true;
    m_irqFile = NULL;
    m_irq = NULL;
    m_checkTimer.setInterval(3000);
    QObject::connect(&m_checkTimer, &QTimer::timeout, this, &Proximity::checkForObject);
}


Proximity::~Proximity()
{
    if (m_irq != NULL)
        delete m_irq;

    if (m_irqFile != NULL) {
        m_irqFile->close();
        delete m_irqFile;
    }
}


void Proximity::setup(int pwm, int pwmChip, int gpio, int period, int dutyCycle)
{
    m_irqGPIO = QString::number(gpio);
    m_pwm = QString::number(pwm);
    m_pwmChip = QString::number(pwmChip);
    m_pwmPeriod = QString::number(period);
    m_pwmDutyCycle = QString::number(dutyCycle);

    QString completePWMPath = m_pwmPath + "pwmchip" + m_pwmChip +  "/pwm" + m_pwm;

    /* export GPIO if necessary */
    if (!QDir(m_gpioPath + "gpio" + m_irqGPIO).exists()) {
        QFile exportGPIOFile(m_gpioPath + "/export");

        if (!exportGPIOFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                m_statusOK = false;
                qWarning() << "Proximity: failed to open gpio export file";
                return;
        }

        exportGPIOFile.write(m_irqGPIO.toLocal8Bit());
        exportGPIOFile.close();
    }

    if (!QDir(m_gpioPath + "gpio" + m_irqGPIO).exists()) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to export gpio";
        return;
    }


    /* export PWM if necessary */
    if (!QDir(completePWMPath).exists()) {
        QFile exportPWMFile(m_pwmPath + "pwmchip" + m_pwmChip + "/export");

        if (!exportPWMFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                m_statusOK = false;
                qWarning() << "Proximity: failed to open pwm export file";
                return;
        }

        exportPWMFile.write(m_pwm.toLocal8Bit());
        exportPWMFile.close();
    }

    if (!QDir(completePWMPath).exists()) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to export pwm: path=" << completePWMPath;
        return;
    }


    /* set GPIO direction to 'in' and edge to 'falling' */
    QFile directionFile(m_gpioPath + "gpio" + m_irqGPIO + "/direction");

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open gpio diretion file";
        return;
    }

    directionFile.write(QString("in").toLocal8Bit());
    directionFile.close();

    QFile edgeFile(m_gpioPath + "gpio" + m_irqGPIO + "/edge");

    if (!edgeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open gpio edge file";
        return;
    }

    edgeFile.write(QString("rising").toLocal8Bit());
    edgeFile.close();


    /* set PWM period to sth. like '27000' and duty cycle to about '4000' */
    QFile periodFile(completePWMPath + "/period");

    if (!periodFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open pwm period file";
        return;
    }

    periodFile.write(m_pwmPeriod.toLocal8Bit());
    periodFile.close();

    QFile dutyCycleFile(completePWMPath + "/duty_cycle");

    if (!dutyCycleFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open pwm duty_cycle file";
        return;
    }

    dutyCycleFile.write(m_pwmDutyCycle.toLocal8Bit());
    dutyCycleFile.close();

    if (m_statusOK) {

        if (m_irq != NULL){
            delete m_irq;
            m_irq = NULL;
        }

        if (m_irqFile != NULL) {
            m_irqFile->close();
            delete m_irqFile;
            m_irqFile = NULL;
        }

        m_irqFile = new QFile(m_gpioPath + "gpio" + m_irqGPIO + "/value");

        m_irqFile->open(QIODevice::ReadOnly | QIODevice::Text);

        m_irq = new QSocketNotifier(m_irqFile->handle(), QSocketNotifier::Exception);

        m_irqFile->readAll();

        m_irq->setEnabled(false);

        QObject::connect(m_irq, &QSocketNotifier::activated, this, &Proximity::objectDetected);
        QObject::connect(m_irq, &QSocketNotifier::activated, this, &Proximity::startPollMode);

        /* start in poll mode to get the first value through polling */
        m_checkTimer.start();
        checkForObject();
    }

    return;
}


void Proximity::enablePWM()
{
    QFile pwmEnableFile(m_pwmPath + "pwmchip" + m_pwmChip +  "/pwm" + m_pwm + "/enable");

    if (!pwmEnableFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open pwm enable file";
        return;
    }

    pwmEnableFile.write(QString("1").toLocal8Bit());
    pwmEnableFile.close();
    qDebug() << "Proximity: enabled pwm";
    return;
}


void Proximity::disablePWM()
{
    QFile pwmEnableFile(m_pwmPath + "pwmchip" + m_pwmChip +  "/pwm" + m_pwm + "/enable");

    if (!pwmEnableFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_statusOK = false;
        qWarning() << "Proximity: failed to open pwm enable file";
        return;
    }

    pwmEnableFile.write(QString("0").toLocal8Bit());
    pwmEnableFile.close();
    qDebug() << "Proximity: disabled pwm";
    return;
}


void Proximity::startPollMode()
{
    m_irq->setEnabled(false);
    disablePWM();
    qDebug() << "Proximity: starting timer for poll mode";
    m_checkTimer.start();
    return;
}


void Proximity::enable()
{
    qDebug() << "Proximity: enable sensor";

    if(m_statusOK) {
        enablePWM();
        m_checkTimer.stop();
        m_irqFile->readAll();
        m_irq->setEnabled(true);
    }
    return;
}


void Proximity::disable()
{
    qDebug() << "Proximity: disable sensor";

    if(m_statusOK) {
        m_checkTimer.stop();
        m_irq->setEnabled(false);
        disablePWM();
    }
    return;
}


void Proximity::checkForObject()
{
    enablePWM();
    m_irqFile->reset();
    QByteArray content = m_irqFile->readAll();
    if (content.length() == 0) {
        /* on zero bytes read
         * retrigger the read again in 100ms
         */
        qDebug() << "Proximity: retriggering read in 100ms";
        QTimer::singleShot(100, this, &Proximity::checkForObject);
        disablePWM();
        return;
    }

    if (content.at(0) == '0') {
        /* object detected:
         * emit objectDetected,
         * keep the timer running
         * and wait until the next check
         */
        emit objectDetected();
        disablePWM();
        return;
    }
    else {
        /* no object detected:
         * disable timer and
         * enable the interrupt
         * i.e. wait for the next interrupt
         */
        qDebug() << "Proximity: no object, switching to irq mode";
        /* no need to call enable PWM here */
        enable();
        return;
    }
}
