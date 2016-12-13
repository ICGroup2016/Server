#include "tcpsock.h"
TcpSock::TcpSock(QObject *parent,int _sockDescript, int _id, QString _name)
    :QObject(parent)
{
    connect(&socket,static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),this,&TcpSock::emitError);
    connect(&socket,&QTcpSocket::readyRead,this,&TcpSock::handleInput);
    if(!socket.setSocketDescriptor(_sockDescript)){
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
    Message message;
    qDebug()<<"start reading\n";
    io>>message;
    qDebug()<<"finished reading\n";
    emit emitMessage(message);
}
bool TcpSock::event(QEvent *e){
    if(e->type()!=QEvent::User)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getReceiverType()==1&&tmp.getReceiverid()==id){
        io<<tmp;
        socket.waitForBytesWritten();
        return true;
    }
    return false;
}
int TcpSock::getID(){
    return id;
}
