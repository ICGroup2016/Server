#include "roomsrv.h"

RoomSrv::RoomSrv(QObject *parent, int _num, int _id)
    : QObject(parent),rt(this,_num,_id)
{
    connect(&rt,&RuntimeWrapper::emitMessage,this,&RoomSrv::processRuntimeMessage);
    connect(&rt,&RuntimeWrapper::finished,&rt,&RuntimeWrapper::deleteLater);
    num=_num;
    ready=0;
    id=_id;
    allowJoin=true;
    allowExit=true;
}
void RoomSrv::returnResult(Message msg, bool res){
    Message response(msg.getType(),msg.getSubtype(),msg.getSenderType(),msg.getSenderid(),msg.getReceiverType(),msg.getReceiverid());
    response.addArgument(res);
    emit emitMessage(res);
}
bool RoomSrv::event(QEvent *e){
    if(e->type()!=(QEvent::Type)2333)
        return QObject::event(e);
    Message tmp=*(Message *)e;
    if(tmp.getType()==1)
        rt.processMessage(tmp);
    else if(tmp.getType()==2){
        switch(tmp.getSubtype()){
        case 0:
            if(tmp.getArgument().isEmpty())
                return false;
            returnResult(tmp,addPlayer(tmp.getSenderid()));
            break;
        case 1:
            ready++;
            sendRoomInfo();
            if(ready==num)
                startGame();
            break;
        case 2:
            if(aboutToStart)
                returnResult(tmp,false);
            else{
                ready--;
                returnResult(tmp,true);
                sendRoomInfo();
            }
            break;
        case 3:
            if(tmp.getArgument().isEmpty())
                return false;
            returnResult(tmp,removePlayer(tmp.getArgument()[0],tmp.getSenderid()));
            break;
        case 7:
            if(tmp.getArgument().isEmpty())
                return false;
            if(inDiscussion&&tmp.getArgument()[0]==0){
                tmp.setType(1);
                tmp.setSubtype(10);
                tmp.setReceiverType(1);
                tmp.setReceiverid(-2);
                redirectMessage(tmp);
            }
            else if(tmp.getArgument()[0]==1){
                speakerCount--;
                rt.stopWaitForPlayer(tmp.getSenderid());
            }
            else if(tmp.getArgument()[0]==-1){
                speakerCount=0;
                rt.stopWaitForPlayer(-1);
            }
            if(speakerCount==0)
                inDiscussion=false;
        }
    }
    return false;
}
void RoomSrv::processRuntimeMessage(Message msg){
    redirectMessage(msg);
    QVector<int> arg=msg.getArgument();
    if(msg.getSubtype()==5)
        openDiscussion(-2,&arg);
    else if(msg.getSubtype()==12)
        openDiscussion();
}
bool RoomSrv::addPlayer(int id){
    if(!allowJoin)
        return false;
    int i=0;
    while(map.contains(i))i++;
    map.insert(i,id);
    sendRoomInfo();
    return true;
}
bool RoomSrv::removePlayer(bool force, int id){
    if(!allowExit&&!force)
        return false;
    Message tmp(2,8);
    tmp.addArgument(0);
    emit emitMessage(tmp);
    map.remove(map.key(id));
    sendRoomInfo();
    return true;
}
void RoomSrv::startGame(){
    allowExit=false;
    allowJoin=false;
    Message tmp(2,4);
    emit emitMessage(tmp);
    tmp.setReceiverType(1);
    tmp.setReceiverid(-1);
    redirectMessage(tmp);
    rt.start();
}
void RoomSrv::redirectMessage(Message msg){
    msg.setSenderType(2);
    msg.setSenderid(id);
    if(msg.getReceiverType()==1){
        if(msg.getReceiverid()==-1)
            for(int i=0;i<num;i++)
                if(map.contains(i)){
                    msg.setReceiverid(map[i]);
                    emit emitMessage(msg);
                }

        else if(msg.getReceiverid()==-2)
            for(int i=0;i<audience.size();i++){
                msg.setReceiverid(map[audience[i]]);
                emit emitMessage(msg);
            }
            else{
                    msg.setReceiverid(map[msg.getReceiverid()]);
                    emit emitMessage(msg);
                }

    }
    else
        emit emitMessage(msg);
}
void RoomSrv::sendRoomInfo(){
    Message info(2,6,1,-1);
    info.addArgument(num);
    info.addArgument(ready);
    for(int i=0;i<num;i++)
        if(map.contains(i))
            info.addArgument(map[i]);
    redirectMessage(info);
    info.setReceiverType(0);
    redirectMessage(info);
}
void RoomSrv::openDiscussion(int receiver, QVector<int> *list){
    if(receiver==-1){
        speakerCount=1;
        audience.clear();
        for(int i=0;i<num;i++)
            if(map.contains(i))
                audience.append(i);
    }
    else{
        speakerCount=list->size();
        audience=*list;
    }
    inDiscussion=true;
}
int RoomSrv::getID(){
    return id;
}
