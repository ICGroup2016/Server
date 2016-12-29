#include "tcpsock.h"
TcpSock::TcpSock(QObject *parent,int _sockDescript, int _id, QString _name)
    :QObject(parent),socket(this)
{
    connect(&socket,static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),this,&TcpSock::emitError);
    connect(&socket,&QTcpSocket::readyRead,this,&TcpSock::handleInput);
    socket.setSocketDescriptor(_sockDescript);
    if(!socket.waitForConnected()){
        emitError();
        return;
    }
    io.setDevice(&socket);
    id=_id;
    name=_name;
    qDebug()<<id<<"connected\n";
}
void TcpSock::emitError(){
    Message message(0,0,0,0,1,id);
    message.setDetail(socket.errorString());
    emit emitMessage(message);
    this->deleteLater();
}
void TcpSock::handleInput(){
    Message message;
    io.startTransaction();
    io>>message;
    io.commitTransaction();
    message.setSenderid(id);
    emit emitMessage(message);
}
bool TcpSock::event(QEvent *e){
    if(e->type()!=(QEvent::Type)2333)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    qDebug()<<"Sending message to"<<tmp.getReceiverid();
    qDebug()<<"Type:"<<tmp.getType()<<"\tSubType"<<tmp.getSubtype();
    QByteArray buff;
    QDataStream out(&buff,QIODevice::WriteOnly);
    out<<tmp;
    socket.write(buff);
    return true;
}
int TcpSock::getID(){
    return id;
}
