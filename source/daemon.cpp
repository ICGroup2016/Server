#include "daemon.h"

Daemon::Daemon(QObject *parent)
    :QTcpServer(parent)
{
    if(this->listen())
    {
        QVector<QString> ipAddress;
            QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
            // use the first non-localhost IPv4 address
            for (int i = 0; i < ipAddressesList.size(); ++i) {
                if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                    ipAddressesList.at(i).toIPv4Address()) {
                    ipAddress.append(ipAddressesList.at(i).toString());
                }
            }
            // if we did not find one, use IPv4 localhost
            if (ipAddress.isEmpty())
                ipAddress.append(QHostAddress(QHostAddress::LocalHost).toString());
            qDebug()<<tr("The server is running on %2 port").arg(this->serverPort());
            qDebug()<<tr("The server can be accessed through the following ip:");
            for(int i=0;i<ipAddress.size();i++)
                qDebug()<<ipAddress[i];
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
    t->start();
    connect(socket,&TcpSock::emitMessage,this,&Daemon::deliverMessage);
    connect(socket,&TcpSock::destroyed,t,&QThread::quit);
    connect(t,&QThread::finished,t,&QThread::deleteLater);
    Message msg(0,3,1,id,0,0);
    msg.addArgument(id);
    deliverMessage(msg);
    announceRoomInfo(id);
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
            if(map[tmp.getSenderid()]==-1)
                addRoom(tmp.getArgument()[0],tmp.getSenderid());
            break;
        case 2:
            announceRoomInfo(tmp.getSenderid());
            break;
        case 4:
            if(tmp.getArgument().size()<2)
                return false;
            if(tmp.getArgument()[1]){
                if(map.contains(tmp.getArgument()[0]))
                    map[tmp.getArgument()[0]]=-1;
                for(int i=0;i<roominfo.size();i++)
                    if(roominfo[i].first==tmp.getSenderid()){
                        roominfo[i].second[1]--;
                        if(roominfo[i].second[1]==0){
                            roominfo.removeAt(i);
                            for(int i=0;i<rooms.size();i++)
                                if(rooms[i].isNull()||rooms[i]->getID()==tmp.getSenderid()){
                                    rooms.removeAt(i);
                                    break;
                                }
                        }
                        break;
                    }
            }
            else{
                map[tmp.getArgument()[0]]=tmp.getSenderid();
                for(int i=0;i<roominfo.size();i++)
                    if(roominfo[i].first==tmp.getSenderid()){
                        roominfo[i].second[1]++;
                        break;
                    }
            }
            announceRoomInfo(-1);
            break;
        }
    }
    return true;
}
void Daemon::onNetworkError(Message msg){
    int id=msg.getSenderid();
    for(int i=0;i<connections.size();i++){
        if(connections[i].isNull()||connections[i]->getID()==id){
            connections.removeAt(i);
        }
    }
    if(map[id]!=-1){
        Message msg(2,3,2,map[id],1,id);
        msg.addArgument(1);
        deliverMessage(msg);
    }
    map.remove(id);
}
void Daemon::addRoom(int num, int own){
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
    QPair<int,QVector<int>> info;
    info.first=id;
    info.second.append(num);
    info.second.append(0);
    roominfo.append(info);
    Message msg(2,0,2,id,1,own);
    deliverMessage(msg);
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
            if(!connections[i].isNull())
                if(connections[i]->getID()==tmp->getReceiverid())
                    QCoreApplication::postEvent(connections[i],tmp);
        break;
    case 2:
        for(int i=0;i<rooms.size();i++)
            if(!rooms[i].isNull())
                if(rooms[i]->getID()==tmp->getReceiverid())
                    QCoreApplication::postEvent(rooms[i],tmp);
        break;
    }
}
QVector<int> Daemon::genRoomInfo(){
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
void Daemon::announceRoomInfo(int receiver){
    Message msg(0,2,1,receiver,0,0);
    msg.setArgument(genRoomInfo());
    if(receiver!=-1){
        deliverMessage(msg);
        return;
    }
    for(int i=0;i<connections.size();i++)
        if(!connections[i].isNull()){
            msg.setReceiverid(connections[i]->getID());
            deliverMessage(msg);
        }
}
