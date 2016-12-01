#ifndef MESSAGE_H
#define MESSAGE_H
#include<QtCore>

class Message:public QEvent
{
public:
    Message(int t=0,int st=0,int rt=0,int ri=0);
    void setType(int t);
    int getType()const;
    void setSubtype(int t);
    int getSubtype()const;
    void setReceiverType(int);
    int getReceiverType()const;
    void setReceiverid(int);
    int getReceiverid()const;
    void setDetail(QString _detail);
    QString getDetail()const;
    void addArgument(int arg);
    void setArgument(QVector<int> arg);
    QVector<int> getArgument()const;
    friend QDataStream& operator <<(QDataStream &,Message &);
    friend QDataStream& operator >>(QDataStream &,Message &);
private:
    int type;
    int subtype;
    int receivertype;
    int receiverid;
    QString detail;
    QVector<int> arguments;
};

#endif // MESSAGE_H
