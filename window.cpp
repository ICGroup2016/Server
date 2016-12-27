#include "window.h"
#include "ui_window.h"
#include <QLabel>

window::window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::window)
{
    ui->setupUi(this);

    QLabel *permanent=new QLabel(this);
    permanent->setFrameStyle(QFrame::Box|QFrame::Sunken);
    permanent->setText("欢迎来到Monitor");
    ui->statusBar->addPermanentWidget(permanent);

}

window::~window()
{
    delete ui;
}

void window::addroom(QString ip,QString number){
    QTableWidgetItem*name=new QTableWidgetItem(ip);
    QTableWidgetItem*num=new QTableWidgetItem(number);

    name->setTextAlignment(Qt::AlignCenter);
    num->setTextAlignment(Qt::AlignCenter);

    ui->roomtableWidget->insertRow(0);
    ui->roomtableWidget->setItem(0,0,name);
    ui->roomtableWidget->setItem(0,1,num);
}

void window::removeall(){
    ui->roomtableWidget->setRowCount(0);
    ui->playerWidget->setRowCount(0);
}

void window::addplayer(QString id){
    QTableWidgetItem*idd=new QTableWidgetItem(id);

    idd->setTextAlignment(Qt::AlignCenter);

    ui->playerWidget->insertRow(0);
    ui->playerWidget->setItem(0,0,idd);
}

void window::setup(QString ip, QString port){
    ui->lineEdit->setText("服务器ip地址："+ip);
    ui->lineEdit_2->setText("端口："+port);
}
