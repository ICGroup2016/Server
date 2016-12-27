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
            if (seats.at(i)->getJob()!=Wolf && seats.at(i)->getLife())
                return true;
        }
    }
    Winner = WolfWin;
    return false;
}

void runtime::Assign()
{
    srand(time(NULL));
    int r;
    QString s;
    QVector<int> temp;
    temp.clear();
    QVector<bool> occupied;
    for (int i = 0;i<player_num;i++) occupied.push_back(false);
    for (int i = 0; i<player_num; i++){
        do{
            r=rand()%player_num;
        }while(occupied.at(r));
        occupied[r] = true;
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
            temp.push_back(4);
            MakeMessage(1,1,i,temp,"您的身份为猎人");
            HunterNo = i;
            seats[i]->setJob(Hunter);
            break;
        default:
            if (r <= (player_num-3)/2+3){
                temp.push_back(1);
                MakeMessage(1,1,i,temp,"您的身份为狼人");
                seats[i]->setJob(Wolf);
            }
            else
            {
                temp.push_back(5);
                MakeMessage(1,1,i,temp,"您的身份为村民");
                seats[i]->setJob(Valliger);
            }
            break;
        }
    }
    occupied.~QVector();
    temp.~QVector();
    s.~QString();
}

QVector<int> runtime::getAlivePlayerList(bool IsDay)
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (PlayerOnline.at(i)){
            if (!IsDay){    //如果不是白天，那么当晚杀死的人暂时视为存活
                if (seats.at(i)->getLife() && !KilledTonight.contains(i))
                    x.push_back(i);
            }
        }
    }
    return x;
}

QVector<int> runtime::getAllWolfs()
{
    QVector<int> x;
    for (int i = 0; i<player_num; i++){
        if (PlayerOnline.at(i)){
            if (seats.at(i)->getLife() && seats.at(i)->getJob()==Wolf && !KilledTonight.contains(i))
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

runtime::runtime(QObject * parent,int num)
    :QObject(parent)
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
        PlayerOnline[i] = true;
    }
}

void runtime::Game()
{
    Assign();     //分配座位号和身份
    qDebug() << "Assign完毕";

    QVector<int> temp;
    QVector<int> VoteProcesser;
    int VoteMax = -1;
    QString s;
    temp.clear();
    VoteProcesser.clear();
    player currentplayer = *(new player());

    while (Check())
    {

        KilledTonight.clear();

        AliveList = getAlivePlayerList(false);
        WolfList = getAllWolfs();

        Explode = false;

        //所有人员闭眼
        MakeMessage(1,2,-1,temp,"天黑请闭眼");

        for (int i = 0; i< WolfList.size(); i++)
        {

            //狼人睁眼
            MakeMessage(1,3,WolfList.at(i),temp,"狼人请睁眼");

            //向狼人提供可杀死玩家列表
            MakeMessage(1,4,WolfList.at(i),AliveList,"请讨论夜间杀死的玩家");

        }//让狼人睁眼，公布可杀玩家列表，开启狼人讨论聊天室

        //设置狼人可发言
        MakeMessage(1,5,0,WolfList);

        emit Wait(-1);

        WhisperResults.clear();
        //逐个请求讨论结果
        for (int i = 0; i<WolfList.size(); i++){
             MakeMessage(1,6,WolfList.at(i),AliveList,"请选择今晚杀死的玩家（狼人选择不一致时以被选择最多的玩家为准，并列最多时以座位号更低的狼人选择的目标为准）");
             emit Wait(i);
        }
        //每个人的讨论结果请求完后，房间调用WhisperResult(int seat);

        for (int i = 1; i<WhisperResults.size(); i++){
            if (WhisperResults.at(i) > WhisperResults.at(0)){
                WhisperResults[i] ^= WhisperResults[0];
                WhisperResults[0] ^= WhisperResults[i];
                WhisperResults[i] ^= WhisperResults[0];
            }
        }

        KilledTonight.push_back(WhisperResults.at(0));
        //杀死狼人讨论结果的玩家

        AliveList = getAlivePlayerList(false);

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
                    emit Wait(WitchNo);
                }
                else if (WhisperResults.at(0) == WitchNo){
                    MakeMessage(1,10,WitchNo,temp,"今晚，你死了，你有一瓶解药，但你不能救自己，就是这样");
                }
                else{
                    MakeMessage(1,7,WitchNo,WhisperResults,"今晚，他死了，你有一瓶解药，是否要救？");
                    emit Wait(WitchNo);
                }
            }
            else MakeMessage(1,10,WitchNo,temp,"这里本来该问你用不用解药的，但你已经没有解药啦！");

            AliveList = getAlivePlayerList(false);

            //毒人
            if (Poison && Day != 0)
            {
                MakeMessage(1,8,WitchNo,AliveList,"你有一瓶毒药，是否要毒？");
                emit Wait(WitchNo);
            }
            else
            if (Day != 0) MakeMessage(1,10,WitchNo,temp,"这里本来应该问你用不用毒药的，但你已经没有毒药啦！");
            else MakeMessage(1,10,WitchNo,temp,"第一个晚上不能毒人");

            if ( !Medicine && !Poison) MakeMessage(1,10,WitchNo,temp,"你已经是个白板啦！哈哈哈哈");

            AliveList = getAlivePlayerList(false);

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
            emit Wait(SeerNo);

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

        //全体睁眼
        MakeMessage(1,3,-1,temp,"天亮了");

        //判断自爆
        if (Explode){
            s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
            MakeMessage(1,10,-1,temp,s);
            //公布死亡情况
            switch(KilledTonight.size()){
            case 0:
                MakeMessage(1,10,-1,temp,"今晚是个平安夜");
                break;
            case 1:
                s = QString("今晚%1号玩家死了").arg(KilledTonight.at(0)+1);
                MakeMessage(1,10,-1,temp,s);
                break;
            case 2:
                s = QString("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1);
                MakeMessage(1,10,-1,temp,s);
                break;
            }
            //确认死亡
            for (int i = 0; i< KilledTonight.size(); i++){
                seats[KilledTonight.at(i)]->setLife(false);
                MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
            }
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            Explode = false;
            ExplodeID = -1;
            continue;
        }

        //首日竞选警长
        if (Day == 1){
            MakeMessage(1,10,-1,temp,"开始竞选警长");

            //判断自爆
            if (Explode){
                s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                MakeMessage(1,10,-1,temp,s);
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                Explode = false;
                ExplodeID = -1;
                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                continue;
            }

            MakeMessage(1,11,-1,temp,"是否竞选警长？");
            emit Wait(-1);

            //判断自爆
            if (Explode){
                s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                MakeMessage(1,10,-1,temp,s);
                MakeMessage(1,17,ExplodeID,temp,"你死了");
                Explode = false;
                ExplodeID = -1;
                MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                continue;
            }

            if (OfficerCandidateList.size()==player_num){
                MakeMessage(1,10,-1,temp,"所有人均竞选警长，无人投票，警徽作废");
                //判断自爆
                if (Explode){
                    s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                    MakeMessage(1,10,-1,temp,s);
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    Explode = false;
                    ExplodeID = -1;
                    continue;
                }
            }
            else{
                s = QString("共有%1名玩家参选，下面请他们依次发言").arg(OfficerCandidateList.size());
                MakeMessage(1,10,-1,temp,s);
                //判断自爆
                if (Explode){
                    s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                    MakeMessage(1,10,-1,temp,s);
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    Explode = false;
                    ExplodeID = -1;
                    MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                    continue;
                }
            }

            do{
                for (int i = 0; i < OfficerCandidateList.size(); i++){
                    MakeMessage(1,12,OfficerCandidateList.at(i),temp,"请发表竞选陈述");
                    emit Wait(OfficerCandidateList.at(i));
                    //判断自爆
                    if (Explode){
                        s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                        MakeMessage(1,10,-1,temp,s);
                        MakeMessage(1,17,ExplodeID,temp,"你死了");
                        ExplodeID = -1;
                        MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                        break;
                    }
                }
                if (Explode) break;

                for (int i = 0; i < player_num; i++){
                    if (seats.at(i)->getLife() && !OfficerCandidateList.contains(i))
                        MakeMessage(1,13,i,OfficerCandidateList,"请投票");
                    //判断自爆
                    if (Explode){
                        s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                        MakeMessage(1,10,-1,temp,s);
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
                    if (OfficerCandidateList.contains(i) && OfficerVotePoll.at(i) >= VoteMax){
                        if (OfficerVotePoll.at(i) > VoteMax){
                            VoteMax = OfficerVotePoll.at(i);
                            VoteProcesser.clear();
                        }
                        VoteProcesser.push_back(i);
                    }
                }
                OfficerCandidateList = VoteProcesser;
                MakeMessage(1,10,-1,temp,"本次投票结果如下：");
                for (int i = 0; i < player_num; i++){
                    if (seats.at(i)->getLife() && !OfficerCandidateList.contains(i)){
                        s = QString("%1号玩家投票给了%2号玩家").arg(i+1).arg(OfficerVoteResults.at(i)+1);
                        MakeMessage(1,10,-1,temp,s);
                    }
                }
                //判断自爆
                if (Explode){
                    s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                    MakeMessage(1,10,-1,temp,s);
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
                    break;
                }
                if (OfficerCandidateList.size() == 1){
                    s = QString("%1号玩家成为警长！").arg(OfficerCandidateList.at(0)+1);
                    MakeMessage(1,10,-1,temp,s);
                }else{
                    MakeMessage(1,10,-1,temp,"出现并列！请在并列最高票者中再次投票（本轮未当选的参选者下轮可投票）");
                }
            }while (OfficerCandidateList.size() != 1);  //持续投票，直到唯一一个警长出现
            if (Explode && ExplodeID == -1) continue;

            if (Explode){
                Explode = false;
                //公布死亡情况
                switch(KilledTonight.size()){
                case 0:
                    MakeMessage(1,10,-1,temp,"今晚是个平安夜");
                    break;
                case 1:
                    s = QString("今晚%1号玩家死了").arg(KilledTonight.at(0)+1);
                    MakeMessage(1,10,-1,temp,s);
                    break;
                case 2:
                    s = QString("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1);
                    MakeMessage(1,10,-1,temp,s);
                    break;
                }
                //确认死亡
                for (int i = 0; i< KilledTonight.size(); i++){
                    seats[KilledTonight.at(i)]->setLife(false);
                    MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
                }
                continue;
            }
        }

        //公布死亡情况
        switch(KilledTonight.size()){
        case 0:
            MakeMessage(1,10,-1,temp,"今晚是个平安夜");
            break;
        case 1:
            s = QString("今晚%1号玩家死了").arg(KilledTonight.at(0)+1);
            MakeMessage(1,10,-1,temp,s);
            break;
        case 2:
            s = QString("今晚%1号玩家和%2号玩家死了").arg(KilledTonight.at(0)+1).arg(KilledTonight.at(1)+1);
            MakeMessage(1,10,-1,temp,s);
            break;
        }
        //确认死亡
        for (int i = 0; i< KilledTonight.size(); i++){
            seats[KilledTonight.at(i)]->setLife(false);
            MakeMessage(1,17,KilledTonight.at(i),temp,"你死了");
        }

        //判断自爆
        if (Explode){
            s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
            MakeMessage(1,10,-1,temp,s);
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            MakeMessage(1,10,-1,temp,"本轮游戏无警长！");
            continue;
        }

        //猎人如不是被毒死的，则发动技能
        if (KilledTonight.contains(HunterNo)){
            if (PoisonTarget != HunterNo){
                MakeMessage(1,18,HunterNo,getAlivePlayerList(true),"请选择带走的对象");
                emit Wait(HunterNo);
            }
        }

        //判断自爆
        if (Explode){
            s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
            MakeMessage(1,10,-1,temp,s);
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            Explode = false;
            continue;
        }

        //首日狼人杀死的死者发表遗言
        if (Day == 1){
            for (int i = 0; i < KilledTonight.size(); i++){

                //判断自爆
                if (Explode){
                    s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
                    MakeMessage(1,10,-1,temp,s);
                    MakeMessage(1,17,ExplodeID,temp,"你死了");
                    ExplodeID = -1;
                    break;
                }

                if (Poison){
                    MakeMessage(1,12,KilledTonight.at(i),temp,"请发表遗言");
                    emit Wait(KilledTonight.at(i));
                }else{
                    if (KilledTonight.at(i) != PoisonTarget){
                        MakeMessage(1,12,KilledTonight.at(i),temp,"请发表遗言");
                        emit Wait(KilledTonight.at(i));
                    }
                }
            }
            if (Explode) continue;
        }

        //刷新存活列表
        AliveList = getAlivePlayerList(true);
        WolfList = getAllWolfs();

        KilledTonight.clear();

        //警长死亡传递警徽
        if (OfficerNo != -1)
            if (!(seats.at(OfficerNo)->getLife())){
                MakeMessage(1,10,-1,temp,"警长死亡");
                MakeMessage(1,14,OfficerNo,AliveList,"你死了，是否要传递警徽？如传递，请选择一名存活玩家");
                emit Wait(OfficerNo);
            }

        //警长发言归票，然后决定发言顺序，并发言
        if (OfficerNo != -1){
            MakeMessage(1,10,-1,temp,"下面请警长归票、发言并决定发言方向");
            MakeMessage(1,15,OfficerNo,AliveList,"请归票，阐述理由，并决定发言方向");
            emit Wait(OfficerNo);
        }else{
            MakeMessage(1,10,-1,temp,"从0号玩家开始发言");
            for (int i = 0; i<AliveList.size(); i++){
                MakeMessage(1,12,AliveList.at(i),temp);
                emit Wait(AliveList.at(i));
            }
        }

        //投票
        VoteCandidate = AliveList;
        do{
            for (int i = 0; i<AliveList.size(); i++){
                MakeMessage(1,16,AliveList.at(i),VoteCandidate,"请在以下玩家中投票");
                emit Wait(AliveList.at(i));
            }
            VoteMax = -1;
            VoteProcesser.clear();
            for (int i = 0; i < player_num; i++){
                if (VoteCandidate.contains(i) && VotePoll.at(i) >= VoteMax){
                    if (VotePoll.at(i) > VoteMax){
                        VoteMax = VotePoll.at(i);
                        VoteProcesser.clear();
                    }
                    VoteProcesser.push_back(i);
                }
            }
            VoteCandidate = VoteProcesser;
        }while (VoteCandidate.size() != 1);

        //广播通知X号玩家死亡
        s = QString("%1号玩家票数最多，死亡！").arg(VoteCandidate.at(0)+1);
        MakeMessage(1,10,-1,temp,s);

        //杀死被投的玩家，该玩家发表遗言
        seats[VoteCandidate.at(0)]->setLife(false);
        MakeMessage(1,17,VoteCandidate.at(0),temp,"你死了");

        //判断自爆
        if (Explode){
            s = QString("%1号玩家狼人自爆！！立即进入黑夜！！").arg(ExplodeID+1);
            MakeMessage(1,10,-1,temp,s);
            MakeMessage(1,17,ExplodeID,temp,"你死了");
            ExplodeID = -1;
            continue;
        }

        MakeMessage(1,12,VoteCandidate.at(0),temp,"请发表遗言");
        emit Wait(VoteCandidate.at(0));

        AliveList = getAlivePlayerList(true);

        //猎人技能
        if (VoteCandidate.at(0) == HunterNo){
            MakeMessage(1,18,HunterNo,AliveList,"请选择带走的对象");
            emit Wait(HunterNo);
        }
    }

    MakeMessage(1,3,-1,temp,"游戏结束！");
    if (Winner){
        MakeMessage(1,10,-1,temp,"村民获胜！");
    }
    else{
        MakeMessage(1,10,-1,temp,"狼人获胜！");
    }

    temp.~QVector();
    s.~QString();
    VoteProcesser.~QVector();
}

void runtime::WhisperResult(int seat){
    if (!WhisperResults.contains(seat)){
        WhisperResults.push_back(seat);
    }
}

void runtime::OfficerCandidate(int candi)
{
    OfficerCandidateList.push_back(candi);
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

void runtime::OfficerElection(int voter, int voted)
{
    OfficerVoteResults[voter] = voted;
    OfficerVotePoll[voted] += 1;
}

void runtime::OfficerPass(int receiver)
{
    QVector<int> temp;
    temp.clear();
    OfficerNo = receiver;
    if (receiver == -1){
        MakeMessage(1,10,-1,temp,"警徽撕毁，此后游戏无警长");
    }else{
        MakeMessage(1,10,-1,temp,QString("警长变为%1号玩家").arg(receiver+1));
    }
    temp.~QVector();
}

void runtime::OfficerDecide(int voted, bool direction)
{
    QVector<int> temp;
    temp.clear();
    VoteResults[OfficerNo] = voted;
    VotePoll[voted]+=3;
    int i = OfficerNo;
    if (direction){
        MakeMessage(1,10,-1,temp,"从警长右侧开始发言");
        i = (i + 1) % player_num;
        do{
            if (seats.at(i)->getLife()){
                MakeMessage(1,12,i,temp);
                emit Wait(i);
            }
            i = (i + 1) % player_num;
        }while(i != OfficerNo);
    }else{
        MakeMessage(1,10,-1,temp,"从警长左侧开始发言");
        i = (i - 1 + player_num) % player_num;
        do{
            if (seats.at(i)->getLife()){
                MakeMessage(1,12,i,temp);
                emit Wait(i);
            }
            i = (i - 1 + player_num) % player_num;
        }while(i != OfficerNo);
    }
    temp.~QVector();
}

void runtime::DayVote(int voter, int voted)
{
    VoteResults[voter] = voted;
    VotePoll[voted]+=2;
}

bool runtime::setExplode(int x)
{
    if (!Explode){
        Explode = true;
        ExplodeID = x;
        seats[x]->setLife(false);
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
        MakeMessage(1,10,-1,temp,QString("%1号猎人死亡开枪杀死了%2号玩家").arg(HunterNo+1).arg(x+1));
        MakeMessage(1,17,x,temp,"你死了");
    }
    temp.~QVector();
}

void runtime::RemovePlayer(int x)
{
    QVector<int> temp;
    temp.clear();
    MakeMessage(1,10,-1,temp,QString("%1号玩家因掉线死亡").arg(x+1));
    if (x == OfficerNo){
        MakeMessage(1,10,-1,temp,"警长掉线死亡，警徽作废");
        OfficerNo = -1;
    }
    PlayerOnline[x] = false;
    temp.~QVector();
}
