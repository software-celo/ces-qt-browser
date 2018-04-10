#ifndef INPUTNOTIFIER_H
#define INPUTNOTIFIER_H

#define QT_NO_DEBUG_OUTPUT 1

#include <QObject>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <glob.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <QMutex>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QEventLoop>

class InputNotifier : public QObject
{
    Q_OBJECT
public:
    explicit InputNotifier(QObject *parent = 0);
    ~InputNotifier();

    void setTimeoutLock(int time);
    void setTimeoutBlank(int time);
    void setLockEnable(bool enable);
    void setBlankEnable(bool enable);

signals:
    void lockTimeout();
    void blankTimeout();
    void unblank();
    void finished();

public slots:
    void start();
    void stop();
    void unlock();
    void cleanUp();
    void inputEvent();

private:
    int* m_fds;
    int m_fdsSize;
    int m_timeoutLock;
    int m_timeoutBlank;
    int m_counter;
    bool m_lockEnabled;
    bool m_blankEnabled;
    void setup();
    bool isCancelled();
    enum state {running, locked, blanked, stopped} m_state;
    QMutex* m_stateMutex;
};

#endif // INPUTNOTIFIER_H
