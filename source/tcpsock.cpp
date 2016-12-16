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
    qDebug()<<_sockDescript<<"connected\n";
}
void TcpSock::emitError(){
    Message message(0,0,0,0);
    message.setDetail(socket.errorString());
    message.addArgument(id);
    emit emitMessage(message);
    this->deleteLater();
}
void TcpSock::handleInput(){
    qDebug()<<"In TcpSock #"<<id<<":handle input";
    Message message;
    io.startTransaction();
    qDebug()<<"start reading";
    io>>message;
    qDebug()<<"finished reading";
    io.commitTransaction();
    emit emitMessage(message);
}
bool TcpSock::event(QEvent *e){
    qDebug()<<"In TcpSock #"<<id<<":get message"<<e->type();
    if(e->type()!=(QEvent::Type)2333)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getReceiverType()==1&&tmp.getReceiverid()==id){
        qDebug()<<"start writing";
        QByteArray buff;
        QDataStream out(&buff,QIODevice::WriteOnly);
        out<<tmp;
        socket.write(buff);
        qDebug()<<"finished writing";
        return true;
    }
    return false;
}
int TcpSock::getID(){
    return id;
}
