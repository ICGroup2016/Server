#include "roomsrv.h"

RoomSrv::RoomSrv(QObject *parent, int _num, int _id)
    : QObject(parent),rt(_num,_id)
{
    connect(&rt,&RuntimeWrapper::emitMessage,this,&RoomSrv::processRuntimeMessage);
    num=_num;
    ready=0;
    id=_id;
    aboutToStart=false;
}
void RoomSrv::returnResult(Message msg, bool res){
    Message response(2,msg.getSubtype(),1,map[msg.getArgument()[0]]);
    response.addArgument(res);
    emit emitMessage(res);
}
bool RoomSrv::event(QEvent *e){
    if(e->type()!=(QEvent::Type)2333)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getType()==0){
        removePlayer(true,tmp.getArgument()[0]);
    }
    else if(tmp.getType()==1)
        rt.processMessage(tmp);
    else if(tmp.getType()==2){
        switch(tmp.getSubtype()){
        case 0:
            returnResult(tmp,addPlayer(tmp.getArgument()[0]));
            break;
        case 1:
            ready++;
            if(ready==num)
                startGame();
            break;
        case 2:
            if(aboutToStart)
                returnResult(tmp,false);
            else{
                ready--;
                returnResult(tmp,true);
            }
            break;
        case 3:
            returnResult(tmp,removePlayer(tmp.getArgument()[0]));
            break;
        }
    }
    return false;
}

void RoomSrv::processRuntimeMessage(Message msg){
    redirectMessage(msg);
}
bool RoomSrv::addPlayer(int id){
    if(map.size()==num)
        return false;
    int i=0;
    while(map.contains(i))i++;
    map.insert(i,id);
    if(map.size()==num){
        Message tmp(2,8);
        tmp.addArgument(1);
        emit emitMessage(tmp);
    }
    Message msg(2,6,1,-1);
    QVector<int> players;
    for(int i=0;i<num;i++)
        if(map.contains(i))
            players.append(map[i]);
    msg.setArgument(players);
    redirectMessage(msg);
    return true;
}
bool RoomSrv::removePlayer(bool force, int id){
    if(aboutToStart&&!force)
        return false;
    Message tmp(2,8);
    tmp.addArgument(0);
    emit emitMessage(tmp);
    Message msg(2,6,1,-1);
    QVector<int> players;
    for(int i=0;i<num;i++)
        if(map.contains(i))
            players.append(map[i]);
    msg.setArgument(players);
    redirectMessage(msg);
    map.remove(map.key(id));
    return true;
}
void RoomSrv::startGame(){
    Message tmp(2,4);
    emit emitMessage(tmp);
    tmp.setReceiverType(1);
    tmp.setReceiverid(-1);
    redirectMessage(tmp);
    rt.start();
}
void RoomSrv::redirectMessage(Message msg){
    if(msg.getReceiverType()==1){
        if(msg.getReceiverid()!=-1){
            msg.setReceiverid(map[msg.getReceiverid()]);
            emit emitMessage(msg);
        }
    }
        else
            for(int i=0;i<num;i++)
                if(map.contains(i)){
                    msg.setReceiverid(map[i]);
                    emit emitMessage(msg);
                }
    /*
    else if(msg.getType()==2)
        switch(msg.getSubtype()){
        case 7:
            for(int i=0;i<audience.size();i++)
                if(map.contains(audience[i])){
                    msg.setReceiverType(1);
                    msg.setReceiverid(map[audience[i]]);
                    emit emitMessage(msg);
                }
        }
    */
}
