#include "runtime.h"
#include <ctime>
#include <cstdlib>
#include <QDebug>

bool runtime::Check()
{
    if (getAllWolfs().size()==0){
        Winner = ValligerWin;
        return false;
    }
    for (int i = 0;i<player_num;i++){
        if (PlayerOnline.at(i)){
            if (seats.at(i)->getJob()!=Wolf && seats.at(i)->getLife() && !KilledTonight.contains(i))
                return true;
        }
    }
    Winner = WolfWin;
    return false;
    /*
    if (getAllWolfs().size()==0){
        Winner = ValligerWin;
        return false;
    }
    if (!seats.at(SeerNo)->getLife() && !seats.at(WitchNo)->getLife()){
        if (player_num>=6 && !seats.at(HunterNo)->getLife()){
            Winner = WolfWin;
            return false;
        }
        if (player_num<6){
            Winner = WolfWin;
            return false;
        }
    }
    for (int i=0;i<player_num;i++){
        if (PlayerOnline.at(i)){
            if (seats.at(i)->getJob()==Valliger && seats.at(i)->getLife() && !KilledTonight.contains(i)){
                return true;
            }
        }
    }
    Winner = WolfWin;
    return false;
    *///屠边Check
}

void runtime::Assign()
{
    srand(time(NULL));
    int r;
    QVector<int> temp;
    QVector<int> occupied;
    for (int i = 0;i<player_num;i++) occupied.push_back(false);
    for (int i = 0; i<player_num; i++){
        do{
            r=rand()%player_num;
        }while(occupied.at(r));
        occupied[r] = true;
        temp.clear();
        switch (r) {
        case 0:
            temp.push_back(3);
            MakeMessage(1,1,i,temp,"您的身份为预言家");
            SeerNo = i;
            seats[i]->setJob(Seer);
            break;
        case 1:
            temp.push_back(2);
            MakeMessage(1,1,i,temp,"您的身份为女巫");
            WitchNo = i;
            seats[i]->setJob(Witch);
            break;
        case 2:
            if (player_num >= 6){
                temp.push_back(4);
                MakeMessage(1,1,i,temp,"您的身份为猎人");
                HunterNo = i;
                seats[i]->setJob(Hunter);
                break;
            }
            else
            {
                temp.push_back(1);
                MakeMessage(1,1,i,temp,"您的身份为狼人");
                seats[i]->setJob(Wolf);
            }
        default:
            if (r <= (player_num-(player_num >= 6? 3:2))/2+ (player_num >= 6? 3:2)){
                temp.push_back(1);
                MakeMessage(1,1,i,temp,"您的身份为狼人");
                seats[i]->setJob(Wolf);
            }
            else
            {
                temp.push_back(5);
                MakeMessage(1,1,i,temp,"您的身份为平民");
                seats[i]->setJob(Valliger);
            }
            break;
        }
    }
}

QVector<int> runtime::getAlivePlayerList()
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (PlayerOnline.at(i)){
                if (seats.at(i)->getLife()) x.push_back(i);
        }
    }
    return x;
}

QVector<int> runtime::getAllWolfs()
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (PlayerOnline.at(i)){
            if (seats.at(i)->getLife() && seats.at(i)->getJob()==Wolf)
                x.push_back(i);
        }
    }
    return x;
}

void runtime::MakeMessage(int t, int subt, int recid, QVector<int> arg, QString det){
    if (recid == -1 || PlayerOnline.at(recid)){
        CurrentMessage.setType(t);
        CurrentMessage.setSubtype(subt);
        CurrentMessage.setReceiverType(1);
        CurrentMessage.setReceiverid(recid);
        CurrentMessage.setArgument(arg);
        CurrentMessage.setDetail(det);
        emit SendMessage(CurrentMessage);
    }
}

bool runtime::xxor(bool a, bool b)
{
    return ((!a)&&(b))||((!b)&&(a));
}

runtime::runtime(QObject * parent,int num):QObject(parent)
{
    player_num = num;
    Poison = true;
    Medicine = true;
    SeerNo = -1;
    OfficerNo = -1;
    WitchNo = -1;
    HunterNo = -1;
    Day = 0;
    Explode = false;
    ExplodeID = -1;
    player * p;
    for (int i = 0; i < num; i++){
        p = new player();
        seats.push_back(p);
        PlayerOnline.push_back(true);
    }
}

void runtime::Game()
{
    Assign();     //分配座位号和身份

    QVector<int> temp;
    QVector<int> OfficerCandidateListTemp;
    QVector<int> VoteProcesser;
    int VoteMax = -1;
    int round;
    temp.clear();
    VoteProcesser.clear();

    while (Check())
    {

        KilledTonight.clear();

        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        Explode = true;

        //所有人员闭眼
        MakeMessage(1,2,-1,temp,"天黑请闭眼");

        for (int i = 0; i<WolfList.size(); i++)
        {
            //狼人睁眼
            MakeMessage(1,3,WolfList.at(i),temp,"狼人请睁眼");

            //向狼人提供可杀死玩家列表
            MakeMessage(1,4,WolfList.at(i),WolfList,"请讨论夜间杀死的玩家");
        }//让狼人睁眼，公布可杀玩家列表，开启狼人讨论聊天室

        MakeMessage(1,10,-1,temp,"狼人正在讨论，请耐心等待");
        emit Wait(WolfList);

        if (!Check()){ break;}

        WhisperResults.clear();
        //逐个请求讨论结果
        for (int i = 0; i<WolfList.size(); i++){
            if (seats.at(WolfList.at(i))->getLife()){
                MakeMessage(1,6,WolfList.at(i),getAlivePlayerList(),"请选择今晚杀死的玩家（狼人选择不一致时以被选择最多的玩家为准，并列最多时以座位号更低的狼人选择的目标为准）");
                temp.clear();
                temp.push_back(WolfList.at(i));
                emit Wait(temp);
                temp.clear();
            }
        }
        //每个人的讨论结果请求完后，房间调用WhisperResult(int seat);

        if (!Check()){ break;}

        for (int i = 1; i<WhisperResults.size(); i++){
            if (WhisperResults.at(i) > WhisperResults.at(0)){
                WhisperResults[i] ^= WhisperResults[0];
                WhisperResults[0] ^= WhisperResults[i];
                WhisperResults[i] ^= WhisperResults[0];
            }
        }

        if (WhisperResults.size() > 0){
            KilledTonight.push_back(WhisperResults.at(0));
        }
        //杀死狼人讨论结果的玩家

        if (!Check()){ break;}
        for (int i = 0; i < WolfList.size(); i++)
        {
            MakeMessage(1,2,WolfList.at(i),temp,"狼人请闭眼");
        }  //所有狼人闭眼

        if (!Check()){ break;}

        if (seats.at(WitchNo)->getLife())
        {
            //女巫睁眼
            MakeMessage(1,3,WitchNo,temp,"女巫请睁眼");

            //救人，第一晚可以自救
            if (Medicine)
            {
                if (Day == 0 && seats.at(WitchNo)->getLife()){
                    MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
                    temp.clear();
                    temp.push_back(WitchNo);
                    emit Wait(temp);
                    temp.clear();
                }
                else if (WhisperResults.at(0) == WitchNo){
                    MakeMessage(1,10,WitchNo,temp,"今晚，你死了，你有一瓶解药，但你不能救自己，就是这样");
                }
                else if (seats.at(WitchNo)->getLife()){
                    MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
                    temp.clear();
                    temp.push_back(WitchNo);
                    emit Wait(temp);
                    temp.clear();
                }
            }
            else MakeMessage(1,10,WitchNo,temp,"这里本来该问你用不用解药的，但你已经没有解药啦！");

            AliveList = getAlivePlayerList();

            //毒人
            if (Poison && Day != 0 && seats.at(WitchNo)->getLife())
            {
                MakeMessage(1,8,WitchNo,AliveList,"你有一瓶毒药，是否要毒？");
                temp.clear();
                temp.push_back(WitchNo);
                emit Wait(temp);
                temp.clear();
            }
            else
            if (Day != 0) MakeMessage(1,10,WitchNo,temp,"这里本来应该问你用不用毒药的，但你已经没有毒药啦！");
            else MakeMessage(1,10,WitchNo,temp,"第一个晚上不能毒人");

            if ( !Medicine && !Poison) MakeMessage(1,10,WitchNo,temp,"你已经是个白板啦!");

            AliveList = getAlivePlayerList();

            //女巫闭眼
            MakeMessage(1,2,WitchNo,temp,"女巫请闭眼");

        }//关于女巫的操作

        if (seats.at(SeerNo)->getLife())
        {
            //预言家睁眼
            MakeMessage(1,3,SeerNo,temp,"预言家请睁眼");

            //预言家选择观看者
            MakeMessage(1,9,SeerNo,AliveList,"你想要看谁的身份？");
            temp.clear();
            temp.push_back(SeerNo);
            emit Wait(temp);
            temp.clear();

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
            break;
        }//判断是否有人获胜

        Explode = false;

        //全体睁眼
        MakeMessage(1,3,-1,temp,"天亮了");

        //判断自爆
        if (Explode){
            MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
            //公布死亡情况
            switch(KilledTonight.size()){
            case 0:
                MakeMessage(1,10,-1,temp,"今晚是个平安夜");
                break;
            case 1:
                MakeMessage(1,10,-1,temp,tr("今晚%1号玩家死了").arg(KilledTonight.at(0)+1));
                break;
            case 2:
                MakeMessage(1,10,-1,temp,tr("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1));
                break;
            }
            //确认死亡
            for (int i = 0; i< KilledTonight.size(); i++){
                seats[KilledTonight.at(i)]->setLife(false);
                seats[KilledTonight.at(i)]->setDeathDay(Day);
                MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
            }
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            continue;
        }

        //首日竞选警长
        if (Day == 1){
            MakeMessage(1,10,-1,temp,"开始竞选警长");

            //判断自爆
            if (Explode){
                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                ExplodeID = -1;
                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                continue;
            }

            MakeMessage(1,11,-1,temp,"是否竞选警长？");
            emit Wait(getAlivePlayerList());

            //判断自爆
            if (Explode){
                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                ExplodeID = -1;
                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                continue;
            }

            if (OfficerCandidateList.size()==player_num){
                MakeMessage(1,10,-1,temp,"所有人均竞选警长，无人投票，警徽作废");
                //判断自爆
                if (Explode){
                    MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    continue;
                }
            }
            else{
                if (OfficerCandidateList.size()>0){
                    MakeMessage(1,10,-1,temp,tr("共有%1名玩家参选，下面请他们依次发言").arg(OfficerCandidateList.size()));
                    //判断自爆
                    if (Explode){
                        MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                        MakeMessage(1,17,ExplodeID,temp,"你死了");
                        ExplodeID = -1;
                        MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                        continue;
                    }

                    round = 0;
                    do{
                        for (int i = 0; i < OfficerCandidateList.size(); i++){
                            if (seats.at(OfficerCandidateList.at(i))->getLife()){
                                MakeMessage(1,12,OfficerCandidateList.at(i),temp,"请发表竞选陈述");
                                temp.clear();
                                temp.push_back(OfficerCandidateList.at(i));
                                emit Wait(temp);
                                temp.clear();
                            }
                            //判断自爆
                            if (Explode){
                                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                                MakeMessage(1,17,ExplodeID,temp,"你死了");
                                ExplodeID = -1;
                                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                                break;
                            }
                        }
                        if (Explode) break;
                        OfficerCandidateListTemp = OfficerCandidateList;
                        for (int i = 0; i < OfficerCandidateListTemp.size(); i++){
                            if (seats.at(OfficerCandidateListTemp.at(i))->getLife()){
                                MakeMessage(1,5,OfficerCandidateListTemp.at(i),temp,"是否仍然参选？");
                                temp.clear();
                                temp.push_back(OfficerCandidateListTemp.at(i));
                                emit Wait(temp);
                                temp.clear();
                            }
                            //判断自爆
                            if (Explode){
                                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                                MakeMessage(1,17,ExplodeID,temp,"你死了");
                                ExplodeID = -1;
                                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                                break;
                            }
                        }
                        if (Explode) break;

                        if (OfficerCandidateList.size()<=1){
                            temp.clear();
                            if (OfficerCandidateList.size()==1){
                                MakeMessage(1,10,-1,temp,tr("只剩1名玩家参选，%1号玩家自动成为警长").arg(OfficerCandidateList.at(0)+1));
                                OfficerNo=OfficerCandidateList.at(0);
                            }else{
                                MakeMessage(1,10,-1,temp,"所有玩家均退选，警徽作废！");
                                OfficerNo=-1;
                            }
                            break;
                        }else{
                            for (int i = 0; i<player_num;i++){
                                OfficerVotePoll[i]=0;
                                OfficerVoteResults[i]=-1;
                            }
                            for (int i = 0; i < player_num; i++){
                                if (seats.at(i)->getLife() && !OfficerCandidateList.contains(i)){
                                    MakeMessage(1,13,i,OfficerCandidateList,"请投票");
                                    temp.clear();
                                    temp.push_back(i);
                                    emit Wait(temp);
                                    temp.clear();
                                }
                                //判断自爆
                                if (Explode){
                                    MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                                    ExplodeID = -1;
                                    MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                                    break;
                                }
                            }
                            if (Explode) break;

                            VoteMax = -1;
                            VoteProcesser.clear();
                            for (int i = 0; i < player_num; i++){
                                if (OfficerCandidateList.contains(i) && OfficerVotePoll[i] >= VoteMax){
                                    if (OfficerVotePoll[i] > VoteMax){
                                        VoteMax = OfficerVotePoll[i];
                                        VoteProcesser.clear();
                                    }
                                    VoteProcesser.push_back(i);
                                }
                            }
                            MakeMessage(1,10,-1,temp,"本次投票结果如下：");
                            for (int i = 0; i < player_num; i++){
                                if (seats.at(i)->getLife() && !OfficerCandidateList.contains(i)){
                                    MakeMessage(1,10,-1,temp,tr("%1号玩家投票给了%2号玩家").arg(i+1).arg(OfficerVoteResults[i]+1));
                                }
                            }
                            OfficerCandidateList = VoteProcesser;
                            //判断自爆
                            if (Explode){
                                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                                MakeMessage(1,17,ExplodeID,temp,"你死了");
                                ExplodeID = -1;
                                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                                break;
                            }
                            if (OfficerCandidateList.size() == 1){
                                MakeMessage(1,10,-1,temp,tr("%1号玩家成为警长！").arg(OfficerCandidateList.at(0)+1));
                                OfficerNo = OfficerCandidateList.at(0);
                            }else{
                                if (round == 0){
                                    MakeMessage(1,10,-1,temp,"出现并列！请在并列最高票者中再次投票（本轮未当选的参选者下轮可投票）");
                                }else{
                                    MakeMessage(1,10,-1,temp,"第二次出现并列！本轮游戏无警长！");
                                }
                            }
                            round++;
                        }
                    }while (OfficerCandidateList.size() > 1 && round < 2);  //持续投票，直到唯一一个警长出现，或者轮到第二轮

                    if (Explode && ExplodeID == -1) continue;

                    if (Explode){
                        //公布死亡情况
                        switch(KilledTonight.size()){
                        case 0:
                            MakeMessage(1,10,-1,temp,"今晚是个平安夜");
                            break;
                        case 1:
                            MakeMessage(1,10,-1,temp,tr("今晚%1号玩家死了").arg(KilledTonight.at(0)+1));
                            break;
                        case 2:
                            MakeMessage(1,10,-1,temp,tr("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1));
                            break;
                        }
                        //确认死亡
                        for (int i = 0; i< KilledTonight.size(); i++){
                            seats[KilledTonight.at(i)]->setLife(false);
                            seats[KilledTonight.at(i)]->setDeathDay(Day);
                            MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
                        }
                        continue;
                    }
                }else{
                    MakeMessage(1,10,-1,temp,"无人竞选警长，本轮游戏无警长！");
                    //判断自爆
                    if (Explode){
                        MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                        MakeMessage(1,17,ExplodeID,temp,"你死了");
                        ExplodeID = -1;
                        continue;
                    }
                }
            }
        }
        //公布死亡情况
        switch(KilledTonight.size()){
        case 0:
            MakeMessage(1,10,-1,temp,"今晚是个平安夜");
            break;
        case 1:
            MakeMessage(1,10,-1,temp,tr("今晚%1号玩家死了").arg(KilledTonight.at(0)+1));
            break;
        case 2:
            MakeMessage(1,10,-1,temp,tr("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1));
            break;
        }
        //确认死亡
        for (int i = 0; i< KilledTonight.size(); i++){
            seats[KilledTonight.at(i)]->setLife(false);
            seats[KilledTonight.at(i)]->setDeathDay(Day);
            //猎人如不是被毒死的，则发动技能
            if (KilledTonight.contains(HunterNo)){
                if (PoisonTarget != HunterNo && PlayerOnline.contains(HunterNo)){
                    MakeMessage(1,18,HunterNo,getAlivePlayerList(),"请选择带走的对象");
                    temp.clear();
                    temp.push_back(HunterNo);
                    emit Wait(temp);
                    temp.clear();
                }
            }
            MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
        }

        //判断自爆
        if (Explode){
            MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
            continue;
        }

        if (!Check()) break;
        //判断自爆
        if (Explode){
            MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            continue;
        }

        //首日狼人杀死的死者发表遗言
        if (Day == 1){
            for (int i = 0; i < KilledTonight.size(); i++){

                //判断自爆
                if (Explode){
                    MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    break;
                }

                if (Poison && PlayerOnline.contains(KilledTonight.at(i))){
                    MakeMessage(1,12,KilledTonight.at(i),temp,"请发表遗言");
                    temp.clear();
                    temp.push_back(KilledTonight.at(i));
                    emit Wait(temp);
                    temp.clear();
                }else{
                    if (KilledTonight.at(i) != PoisonTarget && PlayerOnline.contains(KilledTonight.at(i))){
                        MakeMessage(1,12,KilledTonight.at(i),temp,"请发表遗言");
                        temp.clear();
                        temp.push_back(KilledTonight.at(i));
                        emit Wait(temp);
                        temp.clear();
                    }
                }
            }
            if (Explode) continue;
        }

        //刷新存活列表
        AliveList = getAlivePlayerList();
        WolfList = getAllWolfs();

        KilledTonight.clear();



        //警长死亡传递警徽
        if (OfficerNo != -1)
            if (!(seats.at(OfficerNo)->getLife())){
                MakeMessage(1,10,-1,temp,"警长死亡");
                if (PlayerOnline.contains(OfficerNo)){
                    MakeMessage(1,14,OfficerNo,AliveList,"你死了，是否要传递警徽？如传递，请选择一名存活玩家");
                    temp.clear();
                    temp.push_back(OfficerNo);
                    emit Wait(temp);
                    temp.clear();
                }else{
                    OfficerNo=-1;
                    temp.clear();
                    MakeMessage(1,10,-1,temp,"警长掉线死亡，警徽作废");
                }
            }

        //警长发言归票，然后决定发言顺序，并发言
        if (OfficerNo != -1){
            //判断自爆
            if (Explode){
                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                ExplodeID = -1;
                continue;
            }
            MakeMessage(1,10,-1,temp,"下面请警长归票、发言并决定发言方向");
            if (PlayerOnline.contains(OfficerNo)){
                MakeMessage(1,15,OfficerNo,AliveList,"请归票，阐述理由，并决定发言方向");
                temp.clear();
                temp.push_back(OfficerNo);
                emit Wait(temp);
                temp.clear();
            }
            //判断自爆
            if (Explode){
                MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                ExplodeID = -1;
                continue;
            }
            if (Direc){
                MakeMessage(1,10,-1,temp,"从警长右侧开始发言");

                for (int i = (OfficerNo + 1) % player_num; i != OfficerNo; i = (i + 1) % player_num){
                    //判断自爆
                    if (Explode){
                        MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                        MakeMessage(1,17,ExplodeID,temp,"你死了");
                        ExplodeID = -1;
                        break;
                    }
                    if (seats.at(i)->getLife()){
                        MakeMessage(1,10,-1,temp,tr("请%1号玩家发言").arg(i+1));
                        MakeMessage(1,12,i,temp);
                        temp.clear();
                        temp.push_back(i);
                        emit Wait(temp);
                        temp.clear();
                    }
                }
                if (Explode){
                    continue;
                }
            }else{
                MakeMessage(1,10,-1,temp,"从警长左侧开始发言");
                for (int i = (OfficerNo - 1 + player_num) % player_num; i != OfficerNo; i = (i - 1 + player_num) % player_num){
                    //判断自爆
                    if (Explode){
                        MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                        MakeMessage(1,17,ExplodeID,temp,"你死了");
                        ExplodeID = -1;
                        break;
                    }
                    if (seats.at(i)->getLife()){
                        MakeMessage(1,10,-1,temp,tr("请%1号玩家发言").arg(i+1));
                        MakeMessage(1,12,i,temp);
                        temp.clear();
                        temp.push_back(i);
                        emit Wait(temp);
                        temp.clear();
                    }
                    i = (i - 1 + player_num) % player_num;
                }
                if (Explode) continue;
            }
        }else{
            MakeMessage(1,10,-1,temp,tr("从%1号玩家开始发言").arg(AliveList.at(0)+1));
            for (int i = 0; i<AliveList.size(); i++){
                if (seats.at(AliveList.at(i))->getLife()){
                    MakeMessage(1,12,AliveList.at(i),temp);
                    temp.clear();
                    temp.push_back(AliveList.at(i));
                    emit Wait(temp);
                    temp.clear();
                }
                //判断自爆
                if (Explode){
                    MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    break;
                }
            }
            if (Explode) continue;
        }

        //投票
        AliveList = getAlivePlayerList();
        VoteCandidate = AliveList;
        round = 0;
        do{
            for (int i = 0;i<player_num;i++){
                VotePoll[i]=0;
                VoteResults[i]=0;
            }//清空投票信息
            for (int i = 0; i<AliveList.size(); i++){
                if (seats.at(AliveList.at(i))->getLife() && AliveList.at(i)!=OfficerNo){
                    MakeMessage(1,16,AliveList.at(i),VoteCandidate,"请在以下玩家中投票");
                    temp.clear();
                    temp.push_back(AliveList.at(i));
                    emit Wait(temp);
                    temp.clear();
                }
                //判断自爆
                if (Explode){
                    MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    break;
                }
            }
            if (Explode) break;
            VoteMax = -1;
            VoteProcesser.clear();
            for (int i = 0; i < player_num; i++){
                if (VoteCandidate.contains(i) && VotePoll[i] >= VoteMax){
                    if (VotePoll[i] > VoteMax){
                        VoteMax = VotePoll[i];
                        VoteProcesser.clear();
                    }
                    VoteProcesser.push_back(i);
                }
            }
            VoteCandidate = VoteProcesser;
            if (VoteCandidate.size()!=1){
                temp.clear();
                if (round == 0){
                    MakeMessage(1,10,-1,temp,"出现平票！在并列最多的玩家中重新投票");
                }else{
                    MakeMessage(1,10,-1,temp,"再次出现平票！今天不处决任何人");
                }
            }
            round++;
        }while (VoteCandidate.size() != 1 && round<2);
        if (Explode) continue;

        //广播通知X号玩家死亡
        MakeMessage(1,10,-1,temp,tr("%1号玩家票数最多，死亡！").arg(VoteCandidate.at(0)+1));

        //杀死被投的玩家，该玩家发表遗言
        seats[VoteCandidate.at(0)]->setLife(false);
        seats[VoteCandidate.at(0)]->setDeathDay(Day);
       //猎人技能
        if (VoteCandidate.at(0) == HunterNo && PlayerOnline.contains(HunterNo)){
            MakeMessage(1,18,HunterNo,AliveList,"请选择带走的对象");
            temp.clear();
            temp.push_back(HunterNo);
            emit Wait(temp);
            temp.clear();
        }
        MakeMessage(1,17,VoteCandidate.at(0),temp,"你死了");

        //判断自爆
        if (Explode){
            MakeMessage(1,10,-1,temp,tr("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1));
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            continue;
        }
        if (PlayerOnline.contains(VoteCandidate.at(0))){
            MakeMessage(1,12,VoteCandidate.at(0),temp,"请发表遗言");
            temp.clear();
            temp.push_back(VoteCandidate.at(0));
            emit Wait(temp);
            temp.clear();
        }
        AliveList = getAlivePlayerList();
    }
    MakeMessage(1,3,-1,temp,"游戏结束！");
    if (Winner){
        MakeMessage(1,10,-1,temp,"村民获胜！");
    }
    else{
        MakeMessage(1,10,-1,temp,"狼人获胜！");
    }/*
    VoteMax=-1000;
    VoteProcesser.clear();
    srand(time(NULL));
    temp.clear();
    for (int i=0; i<player_num; i++){
        if (Contribution[i]>VoteMax){
            VoteMax=Contribution[i];
            VoteProcesser.clear();
            VoteProcesser.push_back(i);
        }
        else{
            if (Contribution[i]==VoteMax){
                if (xxor((Winner),(seats.at(VoteProcesser[0])->getJob()==Wolf))){
                    if (xxor((!Winner),(seats.at(i)->getJob()==Wolf))){
                        VoteProcesser.clear();
                        VoteProcesser.push_back(i);
                    }
                    else{
                        if(Winner){
                           if (int(seats.at(i)->getJob())<int(seats.at(VoteProcesser[0])->getJob())){
                                VoteProcesser.clear();
                                VoteProcesser.push_back(i);
                            }
                        }
                        else{
                            if (seats.at(i)->getDeathDay()>seats.at(VoteProcesser[0])->getDeathDay()) {
                                VoteProcesser.clear();
                                VoteProcesser.push_back(i);
                            }
                            else{
                                if (rand()%2){
                                    VoteProcesser.clear();
                                    VoteProcesser.push_back(i);
                                }
                            }
                        }
                    }
                }
                else{
                    if (xxor((!Winner),(seats.at(i)->getJob()==Wolf))){
                        if(Winner){
                            if (int(seats.at(i)->getJob())<int(seats.at(VoteProcesser[0])->getJob())){
                                VoteProcesser.clear();
                                VoteProcesser.push_back(i);
                            }
                        }else{
                            if (seats.at(i)->getDeathDay()>seats.at(VoteProcesser[0])->getDeathDay()){
                                VoteProcesser.clear();
                                VoteProcesser.push_back(i);
                            }else{
                                if (rand()%2){
                                    VoteProcesser.clear();
                                    VoteProcesser.push_back(i);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for (int i=0; i<player_num; i++){
        MakeMessage(1,10,-1,temp,tr("%1号玩家的得分为%2分").arg(i+1).arg(Contribution[i]));
    }
    MakeMessage(1,10,-1,temp,tr("本局游戏的MVP是——%1号玩家！").arg(VoteProcesser.at(0)+1));*/
}

void runtime::WhisperResult(int wolfseat, int seat){
    if (!WhisperResults.contains(seat)){
        WhisperResults.push_back(seat);
    }/*
      switch(seats.at(seat)->getJob()){
      case Wolf:
          Contribution[wolfseat]-=4;
          break;
      case Seer:
          Contribution[wolfseat]+=2;
      case Witch:
          Contribution[wolfseat]+=1;
      case Hunter:
          Contribution[wolfseat]+=1;
      case Valliger:
          Contribution[wolfseat]+=1;
          break;
      }*/
}

void runtime::OfficerCandidate(int candi)
{
    OfficerCandidateList.push_back(candi);
}

void runtime::MedicineResult(int res){
    if (res != -1){
        Medicine = false;
        KilledTonight.clear();/*
          switch(seats.at(res)->getJob()){
          case Wolf:
              Contribution[WitchNo]-=1+(player_num-3)/2-getAllWolfs().size();
              break;
          case Seer:
              Contribution[WitchNo]+=2;
          case Hunter:
              Contribution[WitchNo]+=1;
          case Valliger:
              Contribution[WitchNo]+=player_num-(player_num-3)/2-3-getAlivePlayerList().size()+getAllWolfs().size()+seats.at(WitchNo)->getLife()?1:0+seats.at(SeerNo)->getLife()?1:0;
              break;
          }*/
    }
}

void runtime::PoisonResult(int tar){
    if (tar!=-1){
        Poison = false;
        if (!KilledTonight.contains(tar))
            KilledTonight.push_back(tar);
        PoisonTarget = tar;/*
          switch(seats.at(tar)->getJob()){
          case Wolf:
              Contribution[WitchNo]+=1+(player_num-3)/2-getAllWolfs().size();
              break;
          case Seer:
              Contribution[WitchNo]-=2;
          case Hunter:
              Contribution[WitchNo]-=1;
          case Valliger:
              Contribution[WitchNo]-=player_num-(player_num-3)/2-3-getAlivePlayerList().size()+getAllWolfs().size()+seats.at(WitchNo)->getLife()?1:0+seats.at(SeerNo)->getLife()?1:0;
              break;
          }*/
    }
}

void runtime::SeeResult(int res)
{
    SeeResultIsWolf = (seats.at(res)->getJob() == Wolf);
}

void runtime::OfficerElection(int voter, int voted)
{
    OfficerVoteResults[voter] = voted;
    OfficerVotePoll[voted] += 1;/*
    if (seats.at(voter)->getJob()==Wolf){
        switch (seats.at(voted)->getJob()){
        case Seer:
            Contribution[voter]-=2;
        case Witch:
            Contribution[voter]-=1;
            break;
        }
    }else if (voter == SeerNo){
            switch (seats.at(voted)->getJob()){
            case Witch:
                Contribution[voter]+=3;
            case Hunter:
                Contribution[voter]+=2;
            case Valliger:
                Contribution[voter]+=2;
                break;
            case Wolf:
                Contribution[voter]-=2;
                break;
            }
        }else switch (seats.at(voted)->getJob()){
                case Seer:
                    Contribution[voter]+=2;
                case Witch:
                    Contribution[voter]+=1;
                case Hunter:
                    Contribution[voter]+=1;
                case Valliger:
                    Contribution[voter]+=1;
                    break;
                case Wolf:
                    Contribution[voter]-=4;
                    break;
            }*/
}

void runtime::OfficerPass(int receiver)
{
    QVector<int> temp;
    temp.clear();
    OfficerNo = receiver;
    if (receiver == -1){
        MakeMessage(1,10,-1,temp,"警徽撕毁，此后游戏无警长");
    }else{
        MakeMessage(1,10,-1,temp,tr("警长变为%1号玩家").arg(receiver+1));
    }/*
    if (seats.at(OfficerNo)->getJob()==Wolf){
        switch (seats.at(receiver)->getJob()){
        case Seer:
            Contribution[OfficerNo]-=2;
        case Witch:
            Contribution[OfficerNo]-=2;
            break;
        }
    }else if (OfficerNo == SeerNo){
            switch (seats.at(receiver)->getJob()){
            case Witch:
                Contribution[OfficerNo]+=2;
            case Hunter:
                Contribution[OfficerNo]+=2;
            case Valliger:
                Contribution[OfficerNo]+=1;
                break;
            case Wolf:
                Contribution[OfficerNo]-=5;
                break;
            }
    }else switch(seats.at(receiver)->getJob()){
                    case Seer:
                        Contribution[OfficerNo]+=3;
                    case Witch:
                        Contribution[OfficerNo]+=2;
                    case Hunter:
                        Contribution[OfficerNo]+=2;
                    case Valliger:
                        Contribution[OfficerNo]+=1;
                        break;
                    case Wolf:
                        Contribution[OfficerNo]-=4;
                        break;
                }*/
}

void runtime::OfficerDecide(int voted, bool direction)
{
    QVector<int> temp;
    temp.clear();
    VoteResults[OfficerNo] = voted;
    VotePoll[voted]+=3;
    MakeMessage(1,10,-1,temp,tr("警长归票%1号").arg(voted+1));/*
    if (seats.at(OfficerNo)->getJob()==Wolf){
        switch (seats.at(voted)->getJob()){
        case Seer:
            Contribution[OfficerNo]+=2;
        case Witch:
            Contribution[OfficerNo]+=1;
            break;
        }
    }else if (OfficerNo == SeerNo){
            switch (seats.at(voted)->getJob()){
            case Witch:
                Contribution[OfficerNo]-=2;
            case Hunter:
                Contribution[OfficerNo]-=1;
            case Valliger:
                Contribution[OfficerNo]-=1;
                break;
            case Wolf:
                Contribution[OfficerNo]+=2;
                break;
            }
        }else switch (seats.at(voted)->getJob()){
                case Seer:
                    Contribution[OfficerNo]-=3;
                case Witch:
                    Contribution[OfficerNo]-=2;
                case Hunter:
                    Contribution[OfficerNo]-=2;
                case Valliger:
                    Contribution[OfficerNo]-=2;
                    break;
                case Wolf:
                    Contribution[OfficerNo]+=5;
                    break;
            }*/
    Direc = direction;
}

void runtime::DayVote(int voter, int voted)
{
    VoteResults[voter] = voted;
    VotePoll[voted]+=2;/*
    if (seats.at(voter)->getJob()==Wolf){
        switch (seats.at(voted)->getJob()){
        case Seer:
            Contribution[voter]+=2;
        case Witch:
            Contribution[voter]+=1;
            break;
        }
    }else if (voter == SeerNo){
            switch (seats.at(voted)->getJob()){
            case Witch:
                Contribution[voter]-=3;
            case Hunter:
                Contribution[voter]-=2;
            case Valliger:
                Contribution[voter]-=2;
                break;
            case Wolf:
                Contribution[voter]+=2;
                break;
            }
        }else switch (seats.at(voted)->getJob()){
                case Seer:
                    Contribution[voter]-=2;
                case Witch:
                    Contribution[voter]-=1;
                case Hunter:
                    Contribution[voter]-=1;
                case Valliger:
                    Contribution[voter]-=1;
                    break;
                case Wolf:
                    Contribution[voter]+=4;
                    break;
            }*/
}

bool runtime::setExplode(int x)
{
    if (!Explode){
        Explode = true;
        ExplodeID = x;
        seats[x]->setLife(false);
        seats[x]->setDeathDay(Day);
        return true;
    }
    return false;
}

void runtime::HunterKill(int x)
{
    QVector<int> temp;
    temp.clear();
    if (x != -1){
        seats[x]->setLife(false);
        seats[x]->setDeathDay(Day);
        MakeMessage(1,10,-1,temp,tr("%1号猎人死亡，开枪杀死了%2号玩家").arg(HunterNo+1).arg(x+1));
        MakeMessage(1,17,x,temp,"你死了");/*
        switch(seats.at(x)->getJob()){
        case Wolf:
            Contribution[HunterNo]+=1+(player_num-3)/2-getAllWolfs().size();
            break;
        case Seer:
            Contribution[HunterNo]-=2;
        case Witch:
            Contribution[HunterNo]-=1;
        case Valliger:
            Contribution[HunterNo]-=player_num-(player_num-3)/2-3-getAlivePlayerList().size()+getAllWolfs().size()+seats.at(WitchNo)->getLife()?1:0+seats.at(SeerNo)->getLife()?1:0;
            break;
        }*/
    }
}

void runtime::RemovePlayer(int x)
{
    QVector<int> temp;
    temp.clear();
    if(seats.at(OfficerNo)->getLife()){
        MakeMessage(1,10,-1,temp,tr("%1号玩家因掉线死亡").arg(x+1));
    }
    if (x == OfficerNo && seats.at(OfficerNo)->getLife()){
        MakeMessage(1,10,-1,temp,"警长掉线死亡，警徽作废");
        OfficerNo = -1;
    }
    PlayerOnline[x] = false;
    seats.at(x)->setLife(false);
    seats.at(x)->setDeathDay(Day);
}

void runtime::QuitOfficerElection(int x)
{
    QVector<int> temp;
    temp.clear();
    OfficerCandidateList.removeAll(x);
    MakeMessage(1,10,-1,temp,tr("%1号玩家退出警长竞选").arg(x+1));
}
