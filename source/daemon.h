#ifndef DAEMON_H
#define DAEMON_H
#include <QtCore>
#include <QtNetwork>
#include "message.h"
#include "roomsrv.h"
#include "tcpsock.h"
class Daemon : public QTcpServer
{
public:
    Daemon(QObject *parent=nullptr);
protected:
    void incomingConnection(qintptr) Q_DECL_OVERRIDE;
    bool event(QEvent *) Q_DECL_OVERRIDE;
private:
    void addRoom(int,int);
    void deliverMessage(Message);
    void onNetworkError(Message);
    void announceRoomInfo(int);
    QVector<int> genRoomInfo();
    QVector<QPointer<QThread>> pool;
    QVector<QPointer<TcpSock>> connections;
    QVector<QPointer<RoomSrv>> rooms;
    QVector<QPair<int,QVector<int> > > roominfo;
    QHash<int,int> map;
};

#endif // DAEMON_H
