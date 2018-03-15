#include "inputnotifier.h"


InputNotifier::InputNotifier(QObject *parent) : QObject(parent)
{
    m_stateMutex = new QMutex();

    m_stateMutex->lock();
    m_state = stopped;
    m_counter = 0;
    m_stateMutex->unlock();

    m_timeoutLock = 420;
    m_timeoutBlank = 600;
}


void InputNotifier::setup()
{
    uint /*ret,*/ i;
    glob_t glob_result;

    /* find all devices matching /dev/input/event[0-9]* */
    /*ret = */glob("/dev/input/event[0-9]*", GLOB_ERR|GLOB_NOSORT|GLOB_NOESCAPE, NULL, &glob_result);
//    if (ret)
//        qDebug() << "ERROR glob";  //err(EXIT_FAILURE, "glob");


    /* allocate array for opened file descriptors */
    m_fds = (int*) malloc(sizeof(*m_fds) * (glob_result.gl_pathc+1));
//    if (m_fds == NULL)
//        qDebug() << "error malloc"; //err(EXIT_FAILURE, "malloc");

    /* open devices */
    for (i = 0; i < glob_result.gl_pathc; i++) {
        m_fds[i] = open(glob_result.gl_pathv[i], O_RDONLY|O_NONBLOCK);
//        if (m_fds[i] == -1)
//            qDebug() << "error open"; //err(EXIT_FAILURE, "open `%s'", glob_result.gl_pathv[i]);
    }

    m_fds[i] = -1; /* end of array */
    m_fdsSize = glob_result.gl_pathc+1;
}


void InputNotifier::cleanUp()
{
    if (m_fds != NULL) {
        free((void*) m_fds);
        m_fds = NULL;
    }
}



InputNotifier::~InputNotifier()
{
    cleanUp();
    emit finished();
}


void InputNotifier::start()
{
    char buf[512];
    fd_set readfds;
    int nfds = -1;
    int i, ret;
    struct timeval timeout;

    m_stateMutex->lock();
    m_state = locked;
    m_counter = m_timeoutLock;
    m_stateMutex->unlock();
    setup();
    while(m_state != stopped) {

         FD_ZERO(&readfds);

         /* select(2) might alter the fdset, thus freshly set it
            on every iteration */
         for (i = 0; m_fds[i] != -1; i++) {
             FD_SET(m_fds[i], &readfds);
             nfds = m_fds[i] >= nfds ? m_fds[i] + 1 : nfds;

             /* read everything what's available on this fd */
             while ((ret = read(m_fds[i], buf, sizeof(buf))) > 0)
                 continue; /* read away input */
//             if (ret == -1 && errno != EAGAIN)
//                 qDebug() << "error: read"; //err(EXIT_FAILURE, "read");
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
//             qDebug() << "error on select";
             return; //ERROR on select
         }
         if (ret == 0){
             // Timeout
             switch (m_state) {
                case running:
                                // Check if lock timeout was reached
                                if (m_counter > m_timeoutLock) {
                                    emit lockTimeout();
                                    m_stateMutex->lock();
                                    m_state = locked;
                                    m_stateMutex->unlock();
                                }
                                break;
                case locked:
                                // Check if blank timeout was reached
                                 if (m_counter > m_timeoutBlank) {
                                     emit blankTimeout();
                                     m_stateMutex->lock();
                                     m_state = blanked;
                                     m_stateMutex->unlock();
                                 }
                                break;
                case blanked:   // Reset value back to m_timeoutBlank to prevent overflows and stay >= m_timeoutBlank
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
             // Input event was caught
             switch (m_state) {
                 case running:
                                // Reset idle time
                                m_counter = 0;
                                break;
                 case locked:
                                // Do nothing.
                                m_stateMutex->lock();
                                m_counter = m_timeoutLock;
                                m_stateMutex->unlock();
                                break;
                 case blanked:  // Unblank!
                                emit unblank();
                                m_stateMutex->lock();
                                m_state = locked;
                                m_counter = m_timeoutLock;
                                m_stateMutex->unlock();
                                break;
                 default:       // Do nothing.
                 case stopped:
                     QTimer::singleShot(0, this, &InputNotifier::cleanUp);
                     return;
              }
         }
    }
}


void InputNotifier::unlock()
{
    m_stateMutex->lock();
    m_state = running;
    m_counter = 0;
    m_stateMutex->unlock();
}
