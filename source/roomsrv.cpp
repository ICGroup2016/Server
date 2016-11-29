#include "roomsrv.h"

RoomSrv::RoomSrv(QObject *parent, int _num) : QObject(parent)
{
    rt=new runtime(_num);
    connect(rt,&runtime::SendMessage,this,&RoomSrv::processRuntimeMessage);
    num=_num;
    ready=0;
}
void RoomSrv::processMessage(Message msg){
    QMutexLocker locker(&messageLock);
    if(msg.getReceiverType()==3)
        switch(msg.getSubtype()){
            case 0:
                addPlayer(msg.getArgument()[0]);
                break;
            case 1:
                ready++;
                if(ready==num)
                    startLater();
                break;
            case 2:
                if(aboutToStart)
                    returnResult(msg,false);
                else{
                    ready--;
                    returnResult(msg,true);
                }
                break;
            case 3:
                if(aboutToStart)
                    returnResult(msg,false);
                else{
                    removePlayer(msg.getArgument()[0]);
                    returnResult(msg,true);
                }
                break;
            default:
                break;
        }
}
void RoomSrv::returnResult(Message msg, bool res){
    Message response(2,msg.getSubtype(),1,map[msg.getArgument()[0]]);
    response.addArgument(res);
    emit emitMessage(res);
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
