#include "runtimewrapper.h"

RuntimeWrapper::RuntimeWrapper(QObject *parent, int num, int id)
    :QThread(parent),rt(this,num)
{
    isWaiting=false;
    roomID=id;
    rt.moveToThread(this);
    QObject::connect(&rt,&runtime::SendMessage,this,&RuntimeWrapper::processRuntimeMessage);
}
void RuntimeWrapper::run()
{
    rt.Game();
    Message msg(2,5,3,roomID);
    emit emitMessage(msg);
}
bool RuntimeWrapper::processMessage(Message msg)
{
    if(msg.getType()==1){
        switch(msg.getSubtype()){
        case 6:
            if(msg.getArgument().isEmpty())
                return false;
            rt.WhisperResult(msg.getArgument()[0]);
            break;
        case 7:
            if(msg.getArgument().isEmpty())
                return false;
            rt.MedicineResult(msg.getArgument()[0]);
            break;
        case 8:
            if(msg.getArgument().isEmpty())
                return false;
            rt.PoisonResult(msg.getArgument()[0]);
            break;
        case 9:
            if(msg.getArgument().isEmpty())
                return false;
            rt.SeeResult(msg.getArgument()[0]);
            break;
        case 11:
            if(msg.getArgument().isEmpty())
                return false;
            rt.OfficerCandidate(msg.getArgument()[0]);
            break;
        case 13:
            if(msg.getArgument().size()<2)
                return false;
            rt.OfficerElection(msg.getArgument()[0],msg.getArgument()[1]);
            break;
        case 14:
            if(msg.getArgument().isEmpty())
                return false;
            rt.OfficerPass(msg.getArgument()[0]);
            break;
        case 15:
            if(msg.getArgument().size()<2)
                return false;
            rt.OfficerDecide(msg.getArgument()[0],msg.getArgument()[1]);
            break;
        case 16:
            if(msg.getArgument().isEmpty())
                return false;
            rt.DayVote(msg.getSenderid(),msg.getArgument()[0]);
        case 18:
            if(msg.getArgument().isEmpty())
                return false;
            rt.HunterKill(msg.getArgument()[0]);
        case 19:
            rt.setExplode(msg.getSenderid());
        }
        stopWaitForPlayer(msg.getSenderid());
        return true;
    }
    return false;
}
void RuntimeWrapper::waitForPlayer(int i){
    waitLock.lock();
    if(i!=-1){
        waitList.clear();
        waitList.append(i);
    }
    waitForResponse.wait(&waitLock);
    waitLock.unlock();
}
void RuntimeWrapper::processRuntimeMessage(Message msg){
    if(msg.getSubtype()==5)
        waitList=msg.getArgument();
    emit emitMessage(msg);
}
void RuntimeWrapper::stopWaitForPlayer(int i){
    waitLock.lock();
    if(waitList.contains(i))
        waitList.remove(waitList.indexOf(i));
    if(waitList.isEmpty()||i==-1){
        waitList.clear();
        waitLock.unlock();
        waitForResponse.wakeAll();
    }
    waitLock.unlock();
}
void RuntimeWrapper::playerOffline(int i){
    rt.RemovePlayer(i);
    stopWaitForPlayer(i);
}
