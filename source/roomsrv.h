#ifndef ROOMSRV_H
#define ROOMSRV_H
#include<QtCore>
#include<QtNetwork>
#include"message.h"
#include"tcpsock.h"
#include"runtime.h"
class RoomSrv : public QObject
{
    Q_OBJECT
public:
    explicit RoomSrv(QObject *parent = 0, int _num=0);
    void processMessage(Message msg);
signals:
    void emitMessage(Message);
private slots:
    void processRuntimeMessage(Message);
private:
    void redirectMessage(int, int, Message);
    void addPlayer(int id);
    void removePlayer(int seat);
    void returnResult(Message,bool);
    void startLater();
    int num;
    int ready;
    bool aboutToStart;
    QHash<int,int> map;
    runtime *rt;
    QWaitCondition waitForResponse;
    QMutex messageLock;
};

#endif // ROOMSRV_H
