#ifndef IRQWORKER_H
#define IRQWORKER_H

#define QT_NO_DEBUG_OUTPUT 1

#include <QObject>
#include <QAbstractEventDispatcher>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


class IrqWorker : public QObject
{
    Q_OBJECT
public:
    explicit IrqWorker(QObject *parent = 0);

    void setGPIO(int gpio);
    void sensorIRQ();

signals:
//    void started();
    void finished();
    void objectDetected();

public slots:
    void pause();
    void resume();

private:
    int m_gpio;
    enum state { RUNNING, PAUSED };
    state m_state;
    QMutex* m_stateMutex;
    bool isCancelled();
};

#endif // IRQWORKER_H
