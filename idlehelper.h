#ifndef IDLEHELPER_H
#define IDLEHELPER_H

#include <QTimer>
#include <QTime>
#include <QThread>
#include <inputnotifier.h>
#include <QObject>

class IdleHelper : public QObject
{ Q_OBJECT

public:
  explicit IdleHelper( QObject* parent );

public slots:
    void start();
    void unlock();

signals:
    void blankTimeout();
    void lockTimeout();
    void unblank();
    void reloadBrowser();

private:
    InputNotifier* m_notifier;
    QThread* m_idleThread;
};

#endif // IDLEHELPER_H

