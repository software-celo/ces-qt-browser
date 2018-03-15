#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <unistd.h>

class Backlight : public QObject
{
    Q_OBJECT
    Q_PROPERTY( int brightness READ getBrightness WRITE setBrightness NOTIFY brightnessChanged FINAL )

    // We don't want to be able to change the maxBrightness from QML, hence the read only maxBrightness
    // Further, notice that maxBrightness is not an instance variable.
    Q_PROPERTY( int maxBrightness READ getMaxBrightness NOTIFY maxBrightnessChanged FINAL )

public:
    explicit Backlight(QObject* parent);

    int  getBrightness();
    void setBrightness(int brightness);
    int  getMaxBrightness();
    void setBLEnable(int value);

public slots:
    void lock();
    void unlock();
    void blank();
    void unblank();

signals:
    void brightnessChanged();
    void maxBrightnessChanged();

private:
    int m_maxBrightness;
    int m_brightness;
    int m_lockBrightness;
    int m_unlockBrightness;
    QString m_path;
    QString m_blPath;
    bool m_backlightOK;
    QString m_blEnableGpio;

};


#endif // BACKLIGHT_H

