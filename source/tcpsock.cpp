#include "tcpsock.h"
TcpSock::TcpSock(QObject *parent,int _sockDescript, int _id, QString _name)
    :QObject(parent)
{
    connect(&socket,SIGNAL(QAbstractSocket::error(QAbstractSocket::SocketError socketError)),this,SLOT(emitError()));
    connect(&socket,&QTcpSocket::readyRead,this,&TcpSock::handleInput);
    if(!socket.setSocketDescriptor(_sockDescript)){
        return;
    }
    io.setDevice(&socket);
    id=_id;
    name=_name;
}
void TcpSock::emitError(){
    Message message(0,0,0,0);
    message.setDetail(socket.errorString());
    message.addArgument(0);
    emit emitMessage(message);
}
void TcpSock::handleInput(){
    Message message;
    io>>message;
    emit emitMessage(message);
}
void TcpSock::processMessage(Message msg){
    QMutexLocker locker(&messageLock);
    if(msg.getReceiverType()==1&&msg.getReceiverid()==id){
        io<<msg;
        return;
    }
}
