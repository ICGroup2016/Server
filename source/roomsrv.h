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
    explicit RoomSrv(QObject *parent = 0, int _num=0,int _id=0);
signals:
    void emitMessage(Message);
private slots:
    void processRuntimeMessage(Message);
private:
    void redirectMessage(int, int, Message);
    bool addPlayer(int);
    bool removePlayer(int);
    void returnResult(Message,bool);
    void startLater();
    bool event(QEvent *);
    int num;
    int ready;
    int id;
    bool aboutToStart;
    QHash<int,int> map;
    runtime rt;
    QMutex messageLock;
};

#endif // ROOMSRV_H
