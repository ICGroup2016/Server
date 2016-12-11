#include "runtimewrapper.h"

RuntimeWrapper::RuntimeWrapper(int num,int _ID)
    :rt(num)
{
    isWaiting=false;
    roomID=_ID;
    rt.moveToThread(this);
    QObject::connect(&rt,&runtime::SendMessage,this,&RuntimeWrapper::processRuntimeMessage,Qt::DirectConnection);
}
void RuntimeWrapper::run()
{
    rt.Game();
    Message msg(2,5,3,roomID);
    emit emitMessage(msg);
}
void RuntimeWrapper::processRuntimeMessage(Message msg)
{
    messageLock.lock();
    emit emitMessage(msg);
    if(msg.getSubtype()>=5&&msg.getSubtype()!=10){
        isWaiting=true;
        waitForResponse.wait(&messageLock);
    }
    messageLock.unlock();
}
void RuntimeWrapper::processMessage(Message msg)
{
    if(msg.getType()==1)
        switch(msg.getSubtype()){
        case 6:
            rt.WhisperResult(msg.getArgument()[0]);
            break;
        case 7:
            rt.MedicineResult(msg.getArgument()[0]);
            break;
        case 8:
            rt.PoisonResult(msg.getArgument()[0]);
            break;
        case 9:
            rt.SeeResult(msg.getArgument()[0]);
            break;
        case 11:
            rt.OfficerCandidate(msg.getArgument());
            break;
        case 13:
            rt.OfficerElection(msg.getArgument()[0],msg.getArgument()[1]);
            break;
        case 14:
            rt.OfficerPass(msg.getArgument()[0]);
            break;
        case 15:
            rt.OfficerDecide(msg.getArgument()[0],msg.getArgument()[1]);
            break;
        }
    messageLock.lock();
    if(isWaiting){
        isWaiting=false;
        waitForResponse.wakeAll();
    }
    messageLock.unlock();
}
