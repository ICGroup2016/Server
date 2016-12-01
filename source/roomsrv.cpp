#include "roomsrv.h"

RoomSrv::RoomSrv(QObject *parent, int _num, int _id)
    : QObject(parent),rt(_num)
{
    connect(&rt,&runtime::SendMessage,this,&RoomSrv::processRuntimeMessage);
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
    if(e->type()!=QEvent::User)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getType()==2)
        switch(tmp.getSubtype()){
        case 0:
            returnResult(tmp,addPlayer(tmp.getArgument()[0]));
            break;
        case 1:
            ready++;
            if(ready==num)
                startLater();
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
        case 4:
            Message gameStart(2,4,0,0);
            emit emitMessage(gameStart);
            rt.Game();
            break;
        default:
            break;
        }
}

void RoomSrv::processRuntimeMessage(Message msg){
    if(msg.getReceiverid()!=-1){
        msg.setReceiverid(map[msg.getReceiverid()]);
        emit emitMessage(msg);
    }
    else
        for(int i=0;i<num;i++){
            msg.setReceiverid(map[i]);
            emit emitMessage(msg);
        }
}
bool RoomSrv::addPlayer(int id){
    if(map.size()==num)
        return false;
    int i=0;
    while(map.contains(i))i++;
    for(int j=0;j<num;j++)
        if(map.contains(j)){
            Message tmp(2,0,1,map[j]);
            tmp.addArgument(id);
            emit emitMessage(tmp);
        }
    map.insert(i,id);
    return true;
}
bool RoomSrv::removePlayer(int id){
    if(aboutToStart)
        return false;
    map.remove(map.key(id));
    for(int i=0;i<num;i++)
        if(map.contains(j)){
            Message tmp(2,3,1,map[j]);
            tmp.addArgument(id);
            emit emitMessage(tmp);
        }
    return true;
}
void RoomSrv::startLater(){
    aboutToStart=true;
    Message tmp(2,4,3,id);
    emit emitMessage(tmp);
}
