#include "runtime.h"
#include <ctime>
#include <cstdlib>

bool runtime::Check()
{
    if (getAllWolfs().size()==0){
        Winner = ValligerWin;
        return false;
    }
    for (int i = 0;i<player_num;i++){
        if (seats.at(i)->getJob()!=Wolf && seats.at(i)->getLife())
            return true;
    }
    Winner = WolfWin;
    return false;
}

void runtime::Assign()
{
    srand(time(NULL));
    int r;
    QVector<int> temp;
    temp.clear();
    QVector<bool> occupied;
    for (int i = 0;i<player_num;i++) occupied.push_back(false);
    for (int i = 0; i<player_num; i++){
        temp.push_back(i);
        MakeMessage(1,0,i,temp);
        temp.clear();
        do{
            r=rand()%player_num;
        }while(occupied.at(r));
        switch (r) {
        case 0:
            temp.push_back(3);
            MakeMessage(1,1,i,temp,"您的身份为预言家");
            occupied[r] = true;
            SeerNo = i;
            seats.at(i)->setJob(Seer);
            break;
        case 1:
            temp.push_back(2);
            MakeMessage(1,1,i,temp,"您的身份为女巫");
            occupied[r] = true;
            WitchNo = i;
            seats.at(i)->setJob(Witch);
            break;
        case 2:
            temp.push_back(4);
            MakeMessage(1,1,i,temp,"您的身份为猎人");
            occupied[r] = true;
            HunterNo = i;
            seats.at(i)->setJob(Hunter);
            break;
        default:
            if (r <= (player_num-3)/2+3){
                temp.push_back(1);
                MakeMessage(1,1,i,temp,"您的身份为狼人");
                occupied[r] = true;
                seats.at(i)->setJob(Wolf);
            }
            else
            {
                temp.push_back(5);
                MakeMessage(1,1,i,temp,"您的身份为村民");
                occupied[r] = true;
                seats.at(i)->setJob(Valliger);
            }
            break;
        }
    }
    occupied.~QVector();
    temp.~QVector();
}

QVector<int> runtime::getAlivePlayerList()
{
    QVector<int> x = * new QVector<int>;
    return x;
}

QVector<int> runtime::getAllWolfs()
{
    QVector<int> x = * new QVector<int>;
    return x;
}

void runtime::MakeMessage(int t, int subt, int recid, QVector<int> arg, QString det){
    CurrentMessage.setType(t);
    CurrentMessage.setSubtype(subt);
    CurrentMessage.setReceiverType(1);
    CurrentMessage.setReceiverid(recid);
    CurrentMessage.setArgument(arg);
    CurrentMessage.setDetail(det);
    emit SendMessage(CurrentMessage);
}

runtime::runtime(int num)
{
    player_num = num;
    Poison = true;
    Medicine = true;
    SeerFakeDeath = false;
    WitchFakeDeath = false;
    SeerNo = -1;
    WitchNo = -1;
    HunterNo = -1;
}

int runtime::getPlayer_num(){
    return player_num;
}

void runtime::Game()
{
    Assign();     //分配座位号和身份
    QVector<int> temp;
    QString s;
    temp.clear();
    player currentplayer = *(new player());
    Day = 0;
    while (Check())
    {

        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        //所有人员闭眼
        MakeMessage(1,2,-1,temp,"天黑请闭眼");

        KilledTonight.clear();

        for (int i = 0; i< player_num; i++)
        {
            currentplayer = * seats.at(i);
            if (currentplayer.getLife() && currentplayer.getJob()==Wolf)
            {

                //狼人睁眼
                MakeMessage(1,3,i,temp,"狼人请睁眼");

                //向狼人提供可杀死玩家列表
                MakeMessage(1,4,i,AliveList,"请讨论夜间杀死的玩家");

                //开启客户端狼人讨论窗
                MakeMessage(1,5,i,WolfList);

            }
        }//让狼人睁眼，公布可杀玩家列表，开启狼人讨论聊天室

        do{
            WhisperResults.clear();
            //逐个请求讨论结果
            for (int i = 0; i<player_num; i++){
                if (seats.at(i)->getJob()==Wolf && seats.at(i)->getLife()){
                    currentplayer = * seats.at(i);
                    MakeMessage(1,6,i,AliveList);
                }
            }//每个人的讨论结果请求完后，房间调用WhisperResult(int seat);
        }while (WhisperResults.size()!=1);  //直到狼人讨论结果一致之前都要重新决定


        currentplayer = * seats.at(WhisperResults.at(0));
        currentplayer.setLife(false);
        KilledTonight.push_back(WhisperResults.at(0));
        //杀死狼人讨论结果的玩家

        //防止当天被杀的预言家或女巫无法发动技能，设置FakeDeath标志
        if (WhisperResults.at(0) == SeerNo) SeerFakeDeath = true;
        if (WhisperResults.at(0) == WitchNo) WitchFakeDeath =true;

        //刷新存活列表
        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        for (int i = 0; i < player_num; i++)
        {
            currentplayer = * seats.at(i);
            if (currentplayer.getLife() && currentplayer.getJob()==Wolf){
                MakeMessage(1,2,i,temp,"狼人请闭眼");
            }
        }  //所有狼人闭眼

        currentplayer = * seats.at(WitchNo);
        if (currentplayer.getLife() || WitchFakeDeath)
        {
            //女巫睁眼
            MakeMessage(1,3,WitchNo,temp,"女巫请睁眼");

            //救人，不可自救
            if (WhisperResults.at(0)!=WitchNo && Medicine)
            {
                MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
            }

            //毒人
            if (Poison)
            {
                MakeMessage(1,8,WitchNo,getAlivePlayerList(),"你有一瓶毒药，是否要毒？");
            }

            //女巫闭眼
            MakeMessage(1,2,WitchNo,temp,"女巫请闭眼");

        }//关于女巫的操作

        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        currentplayer = * seats.at(SeerNo);
        if (currentplayer.getLife() || SeerFakeDeath)
        {
            //预言家睁眼
            MakeMessage(1,3,SeerNo,temp,"预言家请睁眼");

            //预言家选择观看者
            MakeMessage(1,9,SeerNo,AliveList,"你想要看谁的身份？");

            //返回预言结果
            if (SeeResultIsWolf){
                MakeMessage(1,11,SeerNo,temp,"他的身份是狼人");
            }
            else{
                MakeMessage(1,10,SeerNo,temp,"他的身份是好人");
            }

            //预言家闭眼
            MakeMessage(1,2,SeerNo,temp,"预言家闭眼");
        }//关于预言家的操作

        Day += 1;

        SeerFakeDeath = false;
        WitchFakeDeath = false;

        if (!Check()){
            MakeMessage(1,3,-1,temp,"游戏结束！");
            if (Winner){
                MakeMessage(1,12,-1,temp,"村民获胜！");
            }
            else{
                MakeMessage(1,12,-1,temp,"狼人获胜！");
            }
            break;
        }//判断是否有人获胜

        //全体睁眼
        MakeMessage(1,3,-1,temp,"天亮了");

        if (Day == 1){
            MakeMessage(1,13,-1,temp,"是否竞选警长？");
            if (CandidateList.size()==player_num){
                MakeMessage(1,12,-1,temp,"所有人均竞选警长，无人投票，警徽作废");
            }
            else{
                MakeMessage(1,12,-1,);
            }
            for (int i = 0; i< CandidateList.size(); i++){
                MakeMessage(1,14,i,temp,"请发表竞选陈述");
            }
            for (int i = 0; i< player_num; i++){
                if (seats.at(i)->getLife())
                    MakeMessage(1,15,i,CandidateList,"请投票");
            }
        }

        //公布死亡情况
        switch(KilledTonight.size()){
        case 0:
            MakeMessage(1,12,-1,temp,"今晚是个平安夜");
            break;
        case 1:
            s = QString("今晚%1号玩家死了").arg(KilledTonight.at(0));
            MakeMessage(1,12,-1,temp,s);
            break;
        case 2:
            s = QString("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)).arg(KilledTonight.at(1));
            MakeMessage(1,12,-1,temp,s);
            break;
        }


    }

}

void runtime::WhisperIsOver(){
    WhisperOver = true;
}

void runtime::WhisperResult(int seat){
    if (!WhisperResults.contains(seat)){
        WhisperResults.push_back(seat);
    }
}

void runtime::TalkIsOver(){
    TalkOver = true;
}

void runtime::Candidate(QVector<bool> candi)
{
    for(int i = 0; i<player_num; i++){
        if (candi.at(i)) CandidateList.push_back(i);
    }
}

void runtime::MedicineResult(bool res){
    if (res){
        Medicine = false;
        (*seats.at(WhisperResults.at(0))).setLife(true);
        KilledTonight.pop_back();
    }
}

void runtime::PoisonResult(int tar){
    if (tar!=-1){
        Poison = false;
        (*seats.at(tar)).setLife(false);
        KilledTonight.push_back(tar);
    }
}

void runtime::SeeResult(int res)
{
    SeeResultIsWolf = WolfList.contains(res);
}
