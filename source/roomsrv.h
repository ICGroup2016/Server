#ifndef ROOMSRV_H
#define ROOMSRV_H
#include<QtCore>
#include<QtNetwork>
#include"message.h"
#include"tcpsock.h"
#include"runtime.h"
#include"runtimewrapper.h"
class RoomSrv : public QObject
{
    Q_OBJECT
public:
    explicit RoomSrv(QObject *parent = 0, int _num=0,int _id=0);
signals:
    void emitMessage(Message);
private:
    void processRuntimeMessage(Message);
    void redirectMessage(Message);
    bool addPlayer(int);
    bool removePlayer(bool force=false, int i=0);
    void returnResult(Message,bool);
    void sendRoomInfo();
    void startGame();
    void openDiscussion(int receiver=-1, QVector<int> *list=nullptr);
    bool event(QEvent *);
    int num;
    int ready;
    int id;
    int speakerCount;
    bool aboutToStart;
    bool inDiscussion;
    QHash<int,int> map;
    QVector<int> audience;
    RuntimeWrapper rt;
    QMutex messageLock;
};

#endif // ROOMSRV_H
