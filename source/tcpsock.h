#ifndef TCPSOCK_H
#define TCPSOCK_H
#include"stdafx.h"
class TcpSock:public QObject
{
    Q_OBJECT
public:
    TcpSock(int sockDescript,int id,QString name);
    void sendMessage(Message msg);
    Message getMessage() const;
signals:
    void emitMessage();
private:
    QTcpSocket *socket;
    Message message;
    QMutex *readLock;
    QMutex *writeLock;
};

#endif // TCPSOCK_H
