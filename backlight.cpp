#include "backlight.h"


Backlight::Backlight(QObject* parent = 0): QObject(parent)
{
    //find valid backlight path
    m_path = "/sys/class/backlight/";
    m_blPath = "/sys/class/gpio/";
    m_blEnableGpio = "34";

    try
    {
        QDir dir(m_path);
        QFileInfoList list = dir.entryInfoList();
        m_path =  list.at(2).absoluteFilePath();
        m_backlightOK = true;
    }
    catch (...)
    {
        m_backlightOK = false;
    }

    m_lockBrightness = 1;
    m_unlockBrightness = 4;

    m_brightness = getBrightness();
    m_maxBrightness = getMaxBrightness();

    setBLEnable(1);

    lock();
}

int Backlight::getMaxBrightness()
{
    if (m_backlightOK == false)
        return 0;

    QFile blFile(m_path + "/max_brightness");

    if (!blFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return 0;
    char buffer[5];
    qint64 linelength = blFile.readLine(buffer, sizeof(buffer));
    if (linelength < 0)
        return 0;
    QString br_max(buffer);
    m_maxBrightness = br_max.toInt();

    return m_maxBrightness;
}


int Backlight::getBrightness()
{
    if (m_backlightOK == false)
        return 0;

    QFile blFile(m_path + "/brightness");

    if (!blFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return 0;
    char buffer[5];
    qint64 linelength = blFile.readLine(buffer, sizeof(buffer));
    if (linelength < 0)
        return 0;
    QString br_max(buffer);
    m_brightness = br_max.toInt();

    return m_brightness;
}

void Backlight::setBrightness(int brightness)
{
    if (m_backlightOK == false)
        return;

    if (brightness < 0 || brightness > m_maxBrightness) {
        if (brightness < 0)
            brightness = 0;
        else
            brightness = m_maxBrightness;
    }

//    qDebug() << "setting brightness to " << brightness;

    QFile blFile(m_path + "/brightness");

    if (!blFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

    blFile.write(QString::number(brightness).toLocal8Bit());
    blFile.close();

    m_brightness = getBrightness();
}

void Backlight::lock()
{
    setBrightness(m_lockBrightness);
}

void Backlight::unlock()
{
    setBrightness(m_unlockBrightness);
}

void Backlight::blank()
{
    setBrightness(0);
    setBLEnable(0);
}

void Backlight::unblank()
{
    setBLEnable(1);
    usleep(100000);
    setBrightness(m_lockBrightness);
}

void Backlight::setBLEnable(int value)
{

    if (value > 1 || value < 0) {
        return;
    }

//  check if gpio is already exported
//  qDebug() << "setBLEnable";

    if (!QDir(m_blPath + "gpio" + m_blEnableGpio).exists()) {
        QFile exportFile(m_blPath + "/export");

        if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

        exportFile.write(m_blEnableGpio.toLocal8Bit());
        exportFile.close();

//      set direction
//      qDebug() << "set direction";
        QFile directionFile(m_blPath + "gpio" + m_blEnableGpio + "/direction");

        if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

        directionFile.write(QString("high").toLocal8Bit());
        directionFile.close();
    }

//  set gpio
//  qDebug() << "set gpio";

    QFile gpioFile(m_blPath + "gpio" + m_blEnableGpio + "/value");

    if (!gpioFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

    gpioFile.write(QString::number(value).toLocal8Bit());
    gpioFile.close();

}
