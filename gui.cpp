#include "gui.h"

gui::gui(QObject *parent)
    :QObject(parent)
{

}

void gui::showwindow(){
    w.show();
}

void gui::addroom(QString id, QString number){
    w.addroom(id,number);
}

void gui::removeall(){
    w.removeall();
}

void gui::hidewindow(){
    w.hide();
}

void gui::setup(QString ip, QString port){
    w.setup(ip,port);
}

void gui::addplayer(QString id, QString abc){
    w.addplayer(id,abc);
}
