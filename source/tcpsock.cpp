#include "tcpsock.h"
TcpSock::TcpSock(QObject *parent,int _sockDescript, int _id, QString _name)
    :QObject(parent)
{
    connect(&socket,SIGNAL(QAbstractSocket::error(QAbstractSocket::SocketError socketError)),this,SLOT(emitError()));
    connect(&socket,&QTcpSocket::readyRead,this,&TcpSock::handleInput);
    if(socket.setSocketDescriptor(_sockDescript)){
        return;
    }
    io.setDevice(&socket);
    id=_id;
    name=_name;
}
void TcpSock::sendMessage(Message msg){
    io<<msg;
}
void TcpSock::emitError(){
    message.setType(0);
    message.setSubtype(0);
    message.setReceiverType(0);
    message.setReceiverid(0);
    message.setDetail(socket.errorString());
    message.addArgument(0);
    emit emitMessage();
}
void TcpSock::handleInput(){
    io>>message;
    emit emitMessage();
}
