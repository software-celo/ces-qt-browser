#include "irqworker.h"

IrqWorker::IrqWorker(QObject *parent) : QObject(parent)
{
    m_state = PAUSED;
    m_gpio = 89;
}

void IrqWorker::resume()
{
    qDebug() << "calling resume in irqworker";
    m_stateMutex->lock();
    if (m_state == PAUSED) {
        m_state = RUNNING;
        qDebug() << "resumed";
    }
    m_stateMutex->unlock();
}

void IrqWorker::pause()
{
    qDebug() << "irqworker pause!";
    m_stateMutex->lock();
    m_state = PAUSED;
    m_stateMutex->unlock();
    return;
}

bool IrqWorker::isCancelled()
{
    auto const dispatcher = QThread::currentThread()->eventDispatcher();
    if (!dispatcher) {
        qCritical() << "thread with no dispatcher";
        return false;
    }
    dispatcher->processEvents(QEventLoop::AllEvents);
    return m_state == PAUSED;
}


void IrqWorker::setGPIO(int gpio)
{
    m_gpio = gpio;
    return;
}


void IrqWorker::sensorIRQ()
{
    m_stateMutex->lock();
    m_state = RUNNING;
    m_stateMutex->unlock();

    qDebug() << "started";

    struct pollfd *poll_fd;
    int irq_fd;
    int pv;
    int bytes_read;
    char buf[2];

    /* set powerfail interrupt to falling edge */
    const char* irqPath = "/sys/class/gpio/gpio89/value";      // ("/sys/class/gpio/gpio" + QString::number(m_gpio) + "/value").toStdString().c_str();
    if ((irq_fd = open(irqPath, O_RDWR)) < 0){
        qDebug() << "IRQ_ERROR: open(value)";
        m_stateMutex->lock();
        m_state = PAUSED;
        m_stateMutex->unlock();
        emit finished();
        return;
    }

    /* initialize the pollfd structure */
    poll_fd = (struct pollfd *) malloc(sizeof(*poll_fd));
    poll_fd->fd = irq_fd;
    poll_fd->events = POLLPRI | POLLERR;
    poll_fd->revents = 0;

    while (!isCancelled()){

        /*
         * call read systemcall on file
         * this has to be called before poll
         */
        bytes_read = read(irq_fd, buf, 1);
        qDebug() << "irqworker - while loop... bytes_read=" << bytes_read;

        /*
         * start poll method with 3 seconds timeout
         * use -1 for endless timeout
         * if using -1 this thread has to be killed from outside to stop the application
         */
        if ((pv = poll(poll_fd, 1, 3000)) < 0){
            qDebug() << "IRQ_ERROR: poll";
            break;
        }
        /* if event i.e. irq occured */
        if (pv > 0){
            qDebug() << "irqworker: objectDetected";
            emit objectDetected();
            /*
             * sleep for 3 seconds as the irq might bounce heavily. we don't need the time
             * accuracy and even more important we don't want the cpu load this would generate here
             */
            QTimer::singleShot(3000, this, &IrqWorker::resume);
            QTimer::singleShot(0, this, &IrqWorker::pause);
        }
    }

    free(poll_fd);

    qDebug() << "finished";
    emit finished();
}
