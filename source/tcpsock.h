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
signals:
    void emitMessage(Message);
private slots:
    void handleInput();
    void emitError();
private:
    bool event(QEvent*);
    int id;
    QString name;
    QTcpSocket socket;
    QDataStream io;
    QMutex messageLock;
};

#endif // TCPSOCK_H
