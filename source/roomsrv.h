#ifndef ROOMSRV_H
#define ROOMSRV_H
#include<QtCore>
#include<QtNetwork>
#include"message.h"
#include"tcpsock.h"

class RoomSrv : public QObject
{
    Q_OBJECT
public:
    explicit RoomSrv(QObject *parent = 0,int wolf=0,int vilgr=0,bool prdictr=0,bool witch=0,bool hntr=0);
    void processMessage(Message msg);
signals:
    void emitMessage(Message);
public slots:
    void processRuntimeMessage();
};

#endif // ROOMSRV_H
