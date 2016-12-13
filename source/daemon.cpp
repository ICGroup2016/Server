#include "daemon.h"

Daemon::Daemon(QObject *parent)
    :QTcpServer(parent)
{
    if(this->listen())
        qDebug()<<tr("The server is listening on %1 ip and %2 port").arg(QHostAddress(QHostAddress::LocalHost).toString()).arg(this->serverPort());
}
void Daemon::incomingConnection(qintptr _descriptr){
    int id=0;
    while(map.contains(id))id++;
    map.insert(id,-1);
    TcpSock *socket=new TcpSock(0,_descriptr,id);
    QThread *t=new QThread();
    socket->moveToThread(t);
    connections.push_back(socket);
    pool.push_back(t);
    connect(socket,&TcpSock::emitMessage,this,&Daemon::deliverMessage);
    connect(socket,&TcpSock::destroyed,t,&QThread::quit);
    connect(t,&QThread::finished,t,&QThread::deleteLater);
    t->start();
}
bool Daemon::event(QEvent *e){
    qDebug()<<"Got message in Daemon:"<<e->type()<<"\n";
    if(e->type()!=(QEvent::Type)2333)
        return QTcpServer::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getType()==0){
        switch(tmp.getSubtype()){
        case 0:
            if(tmp.getArgument().isEmpty())
                return false;
            onNetworkError(tmp.getArgument()[0]);
            break;
        case 1:
            if(tmp.getArgument().size()<2)
                return false;
            addRoom(tmp.getArgument()[0],tmp.getArgument()[1]);
            break;
        case 2:
            if(tmp.getArgument().isEmpty())
                return false;
            Message msg(0,2,1,tmp.getArgument()[0]);
            msg.setArgument(genRoomInfo());
            deliverMessage(msg);
        }
    }
    else if(tmp.getType()==1){

    }
    else if(tmp.getType()==2){
        switch(tmp.getSubtype()){
        case 6:
            if(tmp.getArgument().size()<4)
                return false;
            QVector<int> arg;
            for(int i=3;i<tmp.getArgument().size();i++)
                arg.push_back(tmp.getArgument()[i]);
            dispatchRoomInfo(tmp.getArgument()[0],tmp.getArgument()[1],tmp.getArgument()[2],arg);
        }
    }
    return true;
}
void Daemon::onNetworkError(int id){
    qDebug()<<id<<" network error!\n";
    for(int i=0;i<connections.size();i++)
        if(connections[i]->getID()==id)
            connections.removeAt(i);
    if(map[id]!=-1){
        Message msg(2,3,2,map[id]);
        msg.addArgument(id);
        msg.addArgument(1);
        deliverMessage(msg);
    }
    map.remove(id);
}
void Daemon::addRoom(int num, int own){
    int id=0;
    while(map.key(id,-1)!=-1)
        id++;
    RoomSrv *room=new RoomSrv(0,num,id);
    QThread *t=new QThread();
    room->moveToThread(t);
    rooms.push_back(room);
    pool.push_back(t);
    t->start();
    connect(room,&RoomSrv::emitMessage,this,&Daemon::deliverMessage);
    connect(room,&RoomSrv::destroyed,t,&QThread::quit);
    connect(t,&QThread::finished,t,&QThread::deleteLater);
    Message msg(2,3,2,id);
    msg.addArgument(own);
    deliverMessage(msg);
    qDebug()<<"Room #"<<id<<"added";
}
void Daemon::deliverMessage(Message msg){
    qDebug()<<"Delivering message....";
    Message *tmp=new Message();
    *tmp=msg;
    switch(msg.getReceiverType()){
    case 0:
        QCoreApplication::postEvent(this,tmp);
        break;
    case 1:
        for(int i=0;i<connections.size();i++)
            if(connections[i]->getID()==tmp->getReceiverid())
                QCoreApplication::postEvent(connections[i],tmp);
        break;
    case 2:
        for(int i=0;i<rooms.size();i++)
            if(rooms[i]->getID()==tmp->getReceiverid())
                QCoreApplication::postEvent(rooms[i],tmp);
        break;
    }
}
void Daemon::dispatchRoomInfo(int id, int num, int ready, QVector<int> players){
    qDebug()<<"Dispatching room info....";
    int index=0;
    while(roominfo[index].first!=id)
        index++;
    if(players.isEmpty())
        roominfo.removeAt(index);
    roominfo[index].second=num-players.size();
    for(int i=0;i<players.size();i++)
        map[players[i]]=id;
    QVector<int> diff=(map.values(id).toSet()-players.toList().toSet()).toList().toVector();
    for(int i=0;i<diff.size();i++)
        map[diff[i]]=-1;
    QVector<int> info=genRoomInfo();
    Message msg(0,2,1);
    msg.setArgument(info);
    for(int i=0;i<connections.size();i++)
        if(map[connections[i]->getID()]==-1){
            msg.setReceiverid(connections[i]->getID());
            deliverMessage(msg);
        }
}
QVector<int> Daemon::genRoomInfo(){
    qDebug()<<"Generating room info...";
    QVector<int> result;
    int count=0;
    for(int i=0;i<roominfo.size();i++)
        if(roominfo[i].second!=0){
            result.push_back(roominfo[i].first);
            result.push_back(roominfo[i].second);
            count++;
        }
    result.push_front(count);
    return result;
}