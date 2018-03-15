#ifndef INPUTNOTIFIER_H
#define INPUTNOTIFIER_H

#include <QObject>
//#include <QDebug>
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

class InputNotifier : public QObject
{
    Q_OBJECT
public:
    explicit InputNotifier(QObject *parent = 0);
    ~InputNotifier();

    void unlock();

signals:
    void lockTimeout();
    void blankTimeout();
    void unblank();
    void finished();

public slots:
    void start();
    void cleanUp();

private:
    int* m_fds;
    int m_fdsSize;
    int m_timeoutLock;
    int m_timeoutBlank;
    int m_counter;
    void setup();
    enum state {running, locked, blanked, stopped} m_state;
    QMutex* m_stateMutex;
};

#endif // INPUTNOTIFIER_H
