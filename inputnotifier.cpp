#include "inputnotifier.h"


InputNotifier::InputNotifier(QObject *parent) : QObject(parent)
{
    m_stateMutex = new QMutex();

    m_stateMutex->lock();
    m_state = stopped;
    m_counter = 0;
    m_stateMutex->unlock();

    /* set some huge values here until
     * the final values are set
     */
    m_timeoutLock = 1000;
    m_timeoutBlank = 2000;
    m_lockEnabled = true;
}


void InputNotifier::setTimeoutLock(int time)
{
    if (time < 0)
        return;

    m_stateMutex->lock();
    m_timeoutLock = time;
    if (m_state == locked){
        m_counter = m_timeoutLock;
    }
    else {
        if (m_state == running && m_counter > m_timeoutLock)
            m_counter = 0;
    }
    m_stateMutex->unlock();

    return;
}


void InputNotifier::setTimeoutBlank(int time)
{
    if (time < 0 || time < m_timeoutLock)
        return;

    m_stateMutex->lock();
    m_timeoutBlank = time;
    if (m_state == locked){
        m_counter = m_timeoutLock;
    }
    else {
        if (m_state == running && m_counter > m_timeoutLock)
            m_counter = 0;
    }
    m_stateMutex->unlock();

    return;
}


void InputNotifier::setLockEnable(bool enable)
{
    m_lockEnabled = enable;
    return;
}


void InputNotifier::setBlankEnable(bool enable)
{
    m_blankEnabled = enable;
    return;
}


void InputNotifier::setup()
{
    uint ret, i;
    glob_t glob_result;

    /* find all devices matching /dev/input/event[0-9]* */
    ret = glob("/dev/input/event[0-9]*", GLOB_ERR|GLOB_NOSORT|GLOB_NOESCAPE, NULL, &glob_result);
    if (ret)
        qDebug() << "error(InputNotifier::setup): glob";  //err(EXIT_FAILURE, "glob");


    /* allocate array for opened file descriptors */
    m_fds = (int*) malloc(sizeof(*m_fds) * (glob_result.gl_pathc+1));
    if (m_fds == NULL)
        qDebug() << "error(InputNotifier::setup): malloc"; //err(EXIT_FAILURE, "malloc");

    /* open devices */
    for (i = 0; i < glob_result.gl_pathc; i++) {
        m_fds[i] = open(glob_result.gl_pathv[i], O_RDONLY|O_NONBLOCK);
        if (m_fds[i] == -1)
            qDebug() << "error(InputNotifier::setup): open"; //err(EXIT_FAILURE, "open `%s'", glob_result.gl_pathv[i]);
    }

    m_fds[i] = -1; /* end of array */
    m_fdsSize = glob_result.gl_pathc+1;
    return;
}


void InputNotifier::cleanUp()
{
    if (m_fds != NULL) {
        free((void*) m_fds);
        m_fds = NULL;
    }
    QTimer::singleShot(0, this, &InputNotifier::finished);
    return;
}


InputNotifier::~InputNotifier()
{
    return;
}

bool InputNotifier::isCancelled()
{
    auto const dispatcher = QThread::currentThread()->eventDispatcher();
    if (!dispatcher) {
        qCritical() << "thread with no dispatcher";
        return false;
    }
    dispatcher->processEvents(QEventLoop::AllEvents);
    return m_state == stopped;
}


void InputNotifier::start()
{
    char buf[512];
    fd_set readfds;
    int nfds = -1;
    int i, ret;
    struct timeval timeout;

    m_stateMutex->lock();
    if (!m_earlyStopReq)
        m_state = locked;
    m_counter = m_timeoutLock;
    m_stateMutex->unlock();
    setup();
    while(!isCancelled()) {

         FD_ZERO(&readfds);

         /* select(2) might alter the fdset, thus freshly set it
          * on every iteration
          */
         for (i = 0; m_fds[i] != -1; i++) {
             FD_SET(m_fds[i], &readfds);
             nfds = m_fds[i] >= nfds ? m_fds[i] + 1 : nfds;

             /* read everything what's available on this fd */
             while ((ret = read(m_fds[i], buf, sizeof(buf))) > 0)
                 continue; /* read away input */
             if (ret == -1 && errno != EAGAIN)
                 qDebug() << "error(InputNotifier::start): read"; //err(EXIT_FAILURE, "read");
         }

         /* same for timeout */
         timeout.tv_sec = 5;
         timeout.tv_usec = 0;

         ret = select(nfds, &readfds, NULL, NULL, &timeout);
         m_stateMutex->lock();
         m_counter += 5;
         m_stateMutex->unlock();
         if (ret == -1){
             QTimer::singleShot(0, this, &InputNotifier::cleanUp);
             qDebug() << "error(InputNotifier::start): select";
             return;
         }
         if (ret == 0){
             /* Timeout */
             qDebug() << "state=" << m_state << ", timeout " << m_counter << "secs";
             switch (m_state) {
                case running:
                                if (m_lockEnabled) {
                                    /* Check if lock timeout was reached */
                                    if (m_counter > m_timeoutLock) {
                                        m_stateMutex->lock();
                                        m_state = locked;
                                        m_stateMutex->unlock();
                                        emit lockTimeout();
                                    }
                                }
                                else {
                                    if (m_blankEnabled) {
                                        /* Check if blank timeout was reached */
                                        if (m_counter > m_timeoutBlank) {
                                            m_stateMutex->lock();
                                            m_state = blanked;
                                            m_stateMutex->unlock();
                                            emit blankTimeout();
                                        }
                                    }
                                }
                                break;
                case locked:
                                if (m_blankEnabled) {
                                    /* Check if blank timeout was reached */
                                     if (m_counter > m_timeoutBlank) {
                                         m_stateMutex->lock();
                                         m_state = blanked;
                                         m_stateMutex->unlock();
                                         emit blankTimeout();
                                     }
                                }
                                break;
                case blanked:   /* Reset value back to m_timeoutBlank to prevent overflows and stay >= m_timeoutBlank */
                                m_stateMutex->lock();
                                m_counter = m_timeoutBlank;
                                m_stateMutex->unlock();
                                break;
                default:
                case stopped:
                                QTimer::singleShot(0, this, &InputNotifier::cleanUp);
                                return;
             }
         }
         else {
             /* Input event was caught */
             switch (m_state) {
                 case running:
                                /* Reset idle time */
                                m_counter = 0;
                                break;
                 case locked:
                                /* Do nothing. */
                                m_stateMutex->lock();
                                m_counter = m_timeoutLock;
                                m_stateMutex->unlock();
                                break;
                 case blanked:  /* Unblank! */
                                emit unblank();
                                if (m_lockEnabled) {
                                    m_stateMutex->lock();
                                    m_state = locked;
                                    m_counter = m_timeoutLock;
                                    m_stateMutex->unlock();
                                }
                                else {
                                    m_stateMutex->lock();
                                    m_state = running;
                                    m_counter = 0;
                                    m_stateMutex->unlock();
                                }
                                break;
                 default:       /* Do nothing. */
                 case stopped:
                                QTimer::singleShot(0, this, &InputNotifier::cleanUp);
                                return;
              }
         }
    }
    QTimer::singleShot(0, this, &InputNotifier::cleanUp);
    return;
}


void InputNotifier::unlock()
{
    qDebug() << "inputnotifier: unlock";
    m_stateMutex->lock();
    if (m_state != stopped){
        m_state = running;
        m_counter = 0;
    }
    m_stateMutex->unlock();
    return;
}


void InputNotifier::inputEvent()
{
    qDebug() << "inputnotifier: inputEvent";
    if (m_lockEnabled) {
        m_stateMutex->lock();
        if (m_state == blanked)
            m_state = locked;
        m_counter = m_timeoutLock;
        if (m_state == running)
            m_counter = 0;
        m_stateMutex->unlock();
    }
    else {
        m_stateMutex->lock();
        m_state = running;
        m_counter = 0;
        m_stateMutex->unlock();
    }
    emit unblank();
}


void InputNotifier::stop()
{
    qDebug() << "inputnotifier: stop";
    m_stateMutex->lock();
    m_state = stopped;
    m_earlyStopReq = true;
    m_stateMutex->unlock();
    return;
}
