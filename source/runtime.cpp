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

inline void runtime::Assign()
{
    srand(time(NULL));
    int r;
    QString s;
    QVector<int> temp;
    temp.clear();
    QVector<bool> occupied;
    for (int i = 0;i<player_num;i++) occupied.push_back(false);
    for (int i = 0; i<player_num; i++){
        temp.push_back(i);
        s = QString("您的座位号为%1号").arg(i);
        MakeMessage(1,0,i,temp,s);
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
    s.~QString();
}

QVector<int> runtime::getAlivePlayerList()
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (seats.at(i)->getLife()&& !KilledTonight.contains(i))
            x.push_back(i);
    }
    return x;
}

QVector<int> runtime::getAllWolfs()
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (seats.at(i)->getLife() && seats.at(i)->getJob()==Wolf && !KilledTonight.contains(i))
            x.push_back(i);
    }
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

        KilledTonight.clear();

        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        //所有人员闭眼
        MakeMessage(1,2,-1,temp,"天黑请闭眼");

        for (int i = 0; i< WolfList.size(); i++)
        {

            //狼人睁眼
            MakeMessage(1,3,WolfList.at(i),temp,"狼人请睁眼");

            //向狼人提供可杀死玩家列表
            MakeMessage(1,4,WolfList.at(i),AliveList,"请讨论夜间杀死的玩家");

            //开启客户端狼人讨论窗
            MakeMessage(1,5,WolfList.at(i),WolfList);

        }//让狼人睁眼，公布可杀玩家列表，开启狼人讨论聊天室

        do{
            WhisperResults.clear();
            //逐个请求讨论结果
            for (int i = 0; i<WolfList.size(); i++){
                    MakeMessage(1,6,WolfList.at(i),AliveList);
                }
            //每个人的讨论结果请求完后，房间调用WhisperResult(int seat);
        }while (WhisperResults.size()!=1);  //直到狼人讨论结果一致之前都要重新决定

        KilledTonight.push_back(WhisperResults.at(0));
        //杀死狼人讨论结果的玩家

        for (int i = 0; i < WolfList.size(); i++)
        {
            MakeMessage(1,2,WolfList.at(i),temp,"狼人请闭眼");
        }  //所有狼人闭眼

        currentplayer = * seats.at(WitchNo);
        if (currentplayer.getLife())
        {
            //女巫睁眼
            MakeMessage(1,3,WitchNo,temp,"女巫请睁眼");

            //救人，第一晚可以自救
            if (Medicine)
            {
                if (Day == 0){
                    MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
                }
                else if (WhisperResults.at(0) == WitchNo){
                    MakeMessage(1,10,WitchNo,temp,"今晚，你死了，你有一瓶解药，但你不能救自己，就是这样");
                }
                else MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
            }
            else MakeMessage(1,10,WitchNo,temp,"这里本来该问你用不用解药的，但你已经没有解药啦！");

            //毒人
            if (Poison && Day != 0)
            {
                MakeMessage(1,8,WitchNo,AliveList,"你有一瓶毒药，是否要毒？");
            }
            else
            if (Day != 0) MakeMessage(1,10,WitchNo,temp,"这里本来应该问你用不用毒药的，但你已经没有毒药啦！");
            else MakeMessage(1,10,WitchNo,temp,"第一个晚上不能毒人");

            if ( !Medicine && !Poison) MakeMessage(1,10,WitchNo,temp,"你已经是个白板啦！哈哈哈哈");

            //女巫闭眼
            MakeMessage(1,2,WitchNo,temp,"女巫请闭眼");

        }//关于女巫的操作

        currentplayer = * seats.at(SeerNo);
        if (currentplayer.getLife())
        {
            //预言家睁眼
            MakeMessage(1,3,SeerNo,temp,"预言家请睁眼");

            //预言家选择观看者
            MakeMessage(1,9,SeerNo,AliveList,"你想要看谁的身份？");

            //返回预言结果
            if (SeeResultIsWolf){
                MakeMessage(1,10,SeerNo,temp,"他的身份是狼人");
            }
            else{
                MakeMessage(1,10,SeerNo,temp,"他的身份是好人");
            }

            //预言家闭眼
            MakeMessage(1,2,SeerNo,temp,"预言家闭眼");
        }//关于预言家的操作

        Day += 1;

        if (!Check()){
            MakeMessage(1,3,-1,temp,"游戏结束！");
            if (Winner){
                MakeMessage(1,10,-1,temp,"村民获胜！");
            }
            else{
                MakeMessage(1,10,-1,temp,"狼人获胜！");
            }
            break;
        }//判断是否有人获胜

        //全体睁眼
        MakeMessage(1,3,-1,temp,"天亮了");

        if (Day == 1){
            MakeMessage(1,10,-1,temp,"开始竞选警长");
            MakeMessage(1,11,-1,temp,"是否竞选警长？");
            if (CandidateList.size()==player_num){
                MakeMessage(1,10,-1,temp,"所有人均竞选警长，无人投票，警徽作废");
                OfficerNo = -1;
            }
            else{
                s = QString("共有%1名玩家参选，下面请他们依次发言").arg(CandidateList.size());
                MakeMessage(1,10,-1,temp,s);
            }
            for (int i = 0; i< CandidateList.size(); i++){
                MakeMessage(1,12,CandidateList.at(i),temp,"请发表竞选陈述");
            }
            for (int i = 0; i< player_num; i++){
                if (seats.at(i)->getLife() && !CandidateList.contains(i))
                    MakeMessage(1,13,i,CandidateList,"请投票");
            }
        }

        //公布死亡情况
        switch(KilledTonight.size()){
        case 0:
            MakeMessage(1,10,-1,temp,"今晚是个平安夜");
            break;
        case 1:
            s = QString("今晚%1号玩家死了").arg(KilledTonight.at(0));
            MakeMessage(1,10,-1,temp,s);
            break;
        case 2:
            s = QString("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)).arg(KilledTonight.at(1));
            MakeMessage(1,10,-1,temp,s);
            break;
        }

        for (int i = 0; i< KilledTonight.size(); i++){
            seats[KilledTonight.at(i)]->setLife(false);
            MakeMessage(1,10,KilledTonight.at(i),temp,"你死了");
            if (Day == 1){
                MakeMessage(1,12,KilledTonight.at(i),temp,"请发表遗言");
            }
        }

    }

    temp.~QVector();
    s.~QString();
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

void runtime::Candidate(QVector<int> candi)
{
    CandidateList = candi;
}

void runtime::MedicineResult(bool res){
    if (res){
        Medicine = false;
        KilledTonight.pop_back();
    }
}

void runtime::PoisonResult(int tar){
    if (tar!=-1){
        Poison = false;
        KilledTonight.push_back(tar);
        PoisonTarget = tar;
    }
}

void runtime::SeeResult(int res)
{
    SeeResultIsWolf = WolfList.contains(res);
}
