#ifndef IDLEHELPER_H
#define IDLEHELPER_H

#define QT_NO_DEBUG_OUTPUT 1

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QThread>
#include <inputnotifier.h>
#include <configBackend.h>
#include <proximity.h>

class IdleHelper : public QObject
{ Q_OBJECT

    Q_PROPERTY( bool blankEnable READ getBlankEnable NOTIFY blankEnableChanged FINAL )
    Q_PROPERTY( bool lockEnable READ getLockEnable NOTIFY lockEnableChanged FINAL )

public:
  explicit IdleHelper( QObject* parent, ConfigBackend *cfgBackend );
    ~IdleHelper();

    bool getBlankEnable();
    bool getLockEnable();

public slots:
    void start();
    void unlock();
    void reloadConfig();
    void checkDailyReset();

signals:
    void blankTimeout();
    void lockTimeout();
    void unblank();
    void dailyReset();
    void unlockNotifier();
    void blankEnableChanged();
    void lockEnableChanged();

private:
    ConfigBackend* m_cfgBackend;
    InputNotifier* m_notifier;
    QThread* m_idleThread;
    Proximity* m_proximity;
    QTimer m_resetTimer;
    bool m_dailyReset;
    bool m_resetToday;
    int m_resetTime;
};

#endif // IDLEHELPER_H

