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
}
void TcpSock::emitError(){
    Message message(0,0,0,0,1,id);
    message.setDetail(socket.errorString());
    socket.close();
    emit emitMessage(message);
    this->deleteLater();
}
void TcpSock::handleInput(){
    while(socket.bytesAvailable()){
        Message message;
        io.startTransaction();
        io>>message;
        if(!io.commitTransaction())
            return;
        message.setSenderid(id);
        emit emitMessage(message);
    }
}
bool TcpSock::event(QEvent *e){
    if(e->type()!=(QEvent::Type)2333)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    QByteArray buff;
    QDataStream out(&buff,QIODevice::WriteOnly);
    out<<tmp;
    socket.write(buff);
    while(socket.bytesToWrite()){
        socket.flush();
        socket.waitForBytesWritten();
    }
    return true;
}
int TcpSock::getID(){
    return id;
}
