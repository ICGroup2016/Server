#include "daemon.h"

Daemon::Daemon(QObject *parent)
    :QTcpServer(parent)
{
    if(this->listen())
    {
        QString ipAddress;
            QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
            // use the first non-localhost IPv4 address
            for (int i = 0; i < ipAddressesList.size(); ++i) {
                if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                    ipAddressesList.at(i).toIPv4Address()) {
                    ipAddress = ipAddressesList.at(i).toString();
                    break;
                }
            }
            // if we did not find one, use IPv4 localhost
            if (ipAddress.isEmpty())
                ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
            qDebug()<<tr("The server is running on %1 ip and %2 port").arg(ipAddress).arg(this->serverPort());
    }
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
    Message msg(0,3,1,id,0,0);
    msg.addArgument(id);
    deliverMessage(msg);
}
bool Daemon::event(QEvent *e){
    if(e->type()!=(QEvent::Type)2333)
        return QTcpServer::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getType()==0){
        switch(tmp.getSubtype()){
        case 0:
            onNetworkError(tmp);
            break;
        case 1:
            if(tmp.getArgument().isEmpty())
                return false;
            addRoom(tmp.getArgument()[0],tmp.getSenderid());
            break;
        case 2:
            Message msg(0,2,1,tmp.getSenderid());
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
            dispatchRoomInfo(tmp.getSenderid(),tmp.getArgument()[0],tmp.getArgument()[1],tmp.getArgument()[2],arg);
        }
    }
    return true;
}
void Daemon::onNetworkError(Message msg){
    int id=msg.getSenderid();
    qDebug()<<id<<" network error:"<<msg.getDetail()<<"!\n";
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
    qDebug()<<"Adding room...";
    int id=0;
    while(map.key(id,-1)!=-1)
        id++;
    RoomSrv *room=new RoomSrv(nullptr,num,id);
    QThread *t=new QThread();
    room->moveToThread(t);
    rooms.append(room);
    pool.append(t);
    t->start();
    connect(room,&RoomSrv::emitMessage,this,&Daemon::deliverMessage);
    connect(room,&RoomSrv::destroyed,t,&QThread::quit);
    connect(t,&QThread::finished,t,&QThread::deleteLater);
    Message msg(2,0,2,id,1,own);
    deliverMessage(msg);
    qDebug()<<"Room #"<<id<<"added";
}
void Daemon::deliverMessage(Message msg){
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
void Daemon::dispatchRoomInfo(int id, int num, int playerinside,int ready,  QVector<int> players){
    qDebug()<<"Dispatching room info....";
    int index=0;
    while(index<roominfo.size()&&roominfo[index].first!=id)
        index++;
    if(players.isEmpty()){
        roominfo.removeAt(index);
        rooms.removeAt(index);
    }
    else if(index<roominfo.size()){
        roominfo[index].second.clear();
        roominfo[index].second.append(num);
        roominfo[index].second.append(playerinside);
    }
    else{
        QPair<int,QVector<int>> tmp;
        tmp.first=id;
        tmp.second.append(num);
        tmp.second.append(playerinside);
        tmp.second.append(ready);
        roominfo.append(tmp);
    }
    for(int i=0;i<players.size();i++)
        map[players[i]]=id;
    QVector<int> diff=(map.values(id).toSet()-players.toList().toSet()).toList().toVector();
    for(int i=0;i<diff.size();i++)
        if(map.contains(diff[i]))
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
    for(int i=0;i<roominfo.size();i++){
        result.append(roominfo[i].first);
        result.append(roominfo[i].second);
        count++;
    }
    result.push_front(count);
    return result;
}
