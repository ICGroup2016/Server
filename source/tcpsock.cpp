#include "tcpsock.h"
TcpSock::TcpSock(QObject *parent,int _sockDescript, int _id, QString _name)
    :QObject(parent)
{
    connect(&socket,SIGNAL(QAbstractSocket::error(QAbstractSocket::SocketError socketError)),this,SLOT(emitError()));
    connect(&socket,&QTcpSocket::readyRead,this,&TcpSock::handleInput);
    if(!socket.setSocketDescriptor(_sockDescript)){
        Message tmp;
        emit emitMessage(tmp);
        return;
    }
    io.setDevice(&socket);
    id=_id;
    name=_name;
}
void TcpSock::emitError(){
    Message message(0,0,0,0);
    message.setDetail(socket.errorString());
    message.addArgument(id);
    emit emitMessage(message);
}
void TcpSock::handleInput(){
    Message message;
    io>>message;
    emit emitMessage(message);
}
bool TcpSock::event(QEvent *e){
    if(e->type()!=QEvent::User)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getReceiverType()==1&&tmp.getReceiverid()==id){
        io<<tmp;
        return true;
    }
    return false;
}
