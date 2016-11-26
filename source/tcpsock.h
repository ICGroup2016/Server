#ifndef TCPSOCK_H
#define TCPSOCK_H
#include<QtCore>
#include<QtNetwork>
#include"message.h"

class TcpSock:public QObject
{
    Q_OBJECT
public:
    TcpSock(QObject *parent=0, int _sockDescript=0, int _id=0, QString _name=0);
    Message getMessage() const;
    void processMessage(Message msg);
signals:
    void emitMessage();
private slots:
    void handleInput();
    void emitError();
private:
    void sendMessage(Message msg);
    int id;
    QString name;
    QTcpSocket socket;
    QDataStream io;
    Message message;
    QMutex readLock;
    QMutex writeLock;
};

#endif // TCPSOCK_H
