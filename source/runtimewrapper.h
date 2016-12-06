#ifndef RUNTIMEWRAPPER_H
#define RUNTIMEWRAPPER_H
#include <QtCore>
#include "runtime.h"
#include "message.h"
class RuntimeWrapper : public QThread
{
    Q_OBJECT
public:
    RuntimeWrapper(int,int);
    void processMessage(Message);
signals:
    void emitMessage(Message);
protected:
    void run() Q_DECL_OVERRIDE;
private:
    void processRuntimeMessage(Message);
    int roomID;
    bool isWaiting;
    runtime rt;
    QMutex messageLock;
    QWaitCondition waitForResponse;
};

#endif // RUNTIMEWRAPPER_H
