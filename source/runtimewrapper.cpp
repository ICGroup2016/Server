#include "runtimewrapper.h"

RuntimeWrapper::RuntimeWrapper(QObject *parent, int num, int id)
    :QThread(parent),rt(this,num)
{
    isWaiting=false;
    roomID=id;
    QObject::connect(&rt,&runtime::SendMessage,this,&RuntimeWrapper::emitMessage,Qt::DirectConnection);
    QObject::connect(&rt,&runtime::Wait,this,&RuntimeWrapper::waitForPlayer,Qt::DirectConnection);
}
void RuntimeWrapper::run()
{
    rt.Game();
    Message msg(2,5,1,-1);
    emit emitMessage(msg);
}
bool RuntimeWrapper::processMessage(Message msg)
{
    qDebug()<<"In processMessage";
    if(msg.getType()==1){
        switch(msg.getSubtype()){
        case 6:
            if(msg.getArgument().isEmpty())
                return false;
            rt.WhisperResult(msg.getSenderid(),msg.getArgument()[0]);
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
            if(msg.getArgument()[0])
                rt.OfficerCandidate(msg.getSenderid());
            break;
        case 13:
            if(msg.getArgument().isEmpty())
                return false;
            rt.OfficerElection(msg.getSenderid(),msg.getArgument()[0]);
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
            break;
        case 18:
            if(msg.getArgument().isEmpty())
                return false;
            rt.HunterKill(msg.getArgument()[0]);
            break;
        case 19:
            onExplode(msg.getSenderid());
            break;
        }
        stopWaitForPlayer(msg.getSenderid());
        return true;
    }
    return false;
    qDebug()<<"Out processMessage";
}
void RuntimeWrapper::waitForPlayer(QVector<int> i){
    waitLock.lock();
    waitList=i;
    if(!waitList.isEmpty())
        waitForResponse.wait(&waitLock);
    waitLock.unlock();
}
void RuntimeWrapper::stopWaitForPlayer(int i){
    qDebug()<<"In stopWaitForPlayer";
    waitLock.lock();
    if(waitList.contains(i))
        waitList.remove(waitList.indexOf(i));
    if(waitList.isEmpty()||i==-1){
        waitList.clear();
        waitLock.unlock();
        waitForResponse.wakeAll();
    }
    waitLock.unlock();
    qDebug()<<"out waitForPlayer";
}
void RuntimeWrapper::playerOffline(int i){
    rt.RemovePlayer(i);
    stopWaitForPlayer(i);
}
void RuntimeWrapper::onExplode(int seat){
    if(!rt.setExplode(seat)){
        Message msg(1,19,1,seat);
        msg.addArgument(0);
        emit emitMessage(msg);
    }
    else{
        Message msg(1,19,1,-1);
        msg.addArgument(1);
        emit emitMessage(msg);
        stopWaitForPlayer(-1);
    }
}
