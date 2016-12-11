#include "roomsrv.h"

RoomSrv::RoomSrv(QObject *parent, int _num, int _id)
    : QObject(parent),rt(_num,_id)
{
    connect(&rt,&RuntimeWrapper::emitMessage,this,&RoomSrv::processRuntimeMessage);
    connect(&rt,&RuntimeWrapper::finished,&rt,&RuntimeWrapper::deleteLater);
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
    if(tmp.getType()==1)
        rt.processMessage(tmp);
    else if(tmp.getType()==2){
        switch(tmp.getSubtype()){
        case 0:
            returnResult(tmp,addPlayer(tmp.getArgument()[0]));
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
            returnResult(tmp,removePlayer(tmp.getArgument()[1],tmp.getArgument()[0]));
            break;
        case 7:
            if(inDiscussion&&tmp.getArgument()[0]==0){
                tmp.setType(1);
                tmp.setSubtype(10);
                tmp.setReceiverType(1);
                tmp.setReceiverid(-2);
                redirectMessage(tmp);
            }
            else if(tmp.getArgument()[0]==1)
                speakerCount--;
            else if(tmp.getArgument()[0]==-1)
                speakerCount=0;
            if(speakerCount==0){
                inDiscussion=false;
                Message msg(1,-1);
                rt.processMessage(msg);
            }
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
    sendRoomInfo();
    return true;
}
bool RoomSrv::removePlayer(bool force, int id){
    if(aboutToStart&&!force)
        return false;
    Message tmp(2,8);
    tmp.addArgument(0);
    emit emitMessage(tmp);
    map.remove(map.key(id));
    sendRoomInfo();
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
    emit emitMessage(info);
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
