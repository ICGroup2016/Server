#ifndef RUNTIME_H
#define RUNTIME_H

#define WolfWin false
#define ValligerWin true

#include <QString>
#include <QObject>
#include <QVector>
#include "message.h"

enum Job {All,Wolf,Seer,Witch,Hunter,Valliger};

class player{
private:
    Job occu;
    bool IsAlive;
public:
    player(){
        IsAlive = true;
    }
    void setLife(bool l){
        IsAlive = l;
    }
    void setJob(Job j){
        occu = j;
    }
    bool getLife(){
        return IsAlive;
    }
    Job getJob(){
        return occu;
    }
};

class runtime:public QObject{
    Q_OBJECT
private:
    int player_num; //游戏人数
    int Day;    //天数
    int SeerNo; //预言家座位号
    int WitchNo; //女巫座位号
    int HunterNo; //猎人座位号
    int OfficerNo;  //警长座位号，-1代表警徽被撕
    bool Medicine;  //解药是否已使用
    bool Poison;   //毒药是否已使用
    int PoisonTarget;    //被毒者编号
    int ExplodeID;       //自爆者编号
    QVector<bool> PlayerOnline;       //是否在线
    QVector<int> WhisperResults;  //狼人讨论结果
    QVector<int> AliveList;   //游戏进行中存活玩家的座位号
    QVector<int> WolfList;    //游戏进行中存活狼人的座位号
    QVector<int> KilledTonight;    //某天晚上死亡的玩家座位号列表
    QVector<int> OfficerCandidateList;    //首日竞选警长的玩家列表
    QVector<int> OfficerVotePoll;         //每个人得票数
    QVector<int> OfficerVoteResults;     //警长竞选，谁投了谁
    QVector<int> VoteCandidate;   //可以被投死的玩家
    QVector<int> VoteResults;   //白天投票结果（谁投了谁）
    QVector<int> VotePoll;   //每个人得票数
    QVector <player*> seats;  //与座位号对应的玩家类
    Message CurrentMessage;  //通讯用的消息类
    bool Winner;       //获胜方
    bool Explode;     //是否有人自爆
    bool SeeResultIsWolf;   //预言结果是否为狼人

    bool Check();     //判断游戏结束
    bool CheckWinner();    //返回游戏结果
    void Assign();    //分配身份
    QVector <int> getAlivePlayerList(bool IsDay);   //返回存活玩家座位号列表
    QVector <int> getAllWolfs();        //返回存活狼座位号列表
    void MakeMessage(int t, int subt, int recid, QVector<int> arg, QString det = "");  //制作消息并发送
public:
    runtime(QObject * parent = 0,int num);  //构造函数，确定总人数、狼人人数
    void Game();    //游戏全过程
    void WhisperResult(int seat);  //添加狼人讨论结果
    void OfficerCandidate(QVector<int> candi);   //玩家是否参选警长
    void MedicineResult(bool res);  //女巫解药是否使用
    void PoisonResult(int tar);  //女巫毒药对谁使用
    void SeeResult(int res);  //预言家看谁
    void OfficerElection(int voter, int voted);  //选警长时voter选了voted
    void OfficerPass(int receiver);   //警徽流动
    void OfficerDecide(int voted, bool direction); //警长归票，决定发言顺序，true为警左发言
    void DayVote();   //白天投票
    bool setExplode(int x);  //x号玩家自爆
    void HunterKill(int x);  //猎人带走x号玩家
    void RemovePlayer(int x);    //某玩家断线

signals:
    void SendMessage(Message);
    void Wait(int);
};


#endif // RUNTIME_H

