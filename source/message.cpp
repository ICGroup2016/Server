#include "message.h"

Message::Message(int t, int st, int rt, int ri, int sdt, int si)
    :QEvent((QEvent::Type)2333)
{
    type=t,subtype=st,receivertype=rt,receiverid=ri,sendertype=sdt,senderid=si;
}
void Message::setType(int t){
    type=t;
}
int Message::getType()const{
    return type;
}
void Message::setSubtype(int t){
    subtype=t;
}
int Message::getSubtype()const{
    return subtype;
}
void Message::setDetail(QString _detail){
    detail=_detail;
}
QString Message::getDetail()const{
    return detail;
}
void Message::addArgument(int arg){
    arguments.append(arg);
}
void Message::setArgument(QVector<int> arg){
    arguments=arg;
}
int Message::getReceiverid()const{
    return receiverid;
}
void Message::setReceiverid(int t){
    receiverid=t;
}
int Message::getReceiverType()const{
    return receivertype;
}
void Message::setReceiverType(int t){
    receivertype=t;
}
int Message::getSenderType() const {
    return sendertype;
}
void Message::setSenderType(int type){
    sendertype=type;
}
int Message::getSenderid() const {
    return senderid;
}
void Message::setSenderid(int id){
    senderid=id;
}

QVector<int> Message::getArgument()const{
    return arguments;
}
QDataStream& operator <<(QDataStream &str, Message &msg){
    str<<msg.type<<msg.subtype<<msg.receivertype<<msg.receiverid<<msg.sendertype<<msg.senderid<<msg.detail<<msg.arguments;
    return str;
}
QDataStream& operator >>(QDataStream &str, Message &msg){
    str>>msg.type>>msg.subtype>>msg.receivertype>>msg.receiverid>>msg.sendertype>>msg.senderid>>msg.detail>>msg.arguments;
    return str;
}
