#include <QCoreApplication>
#include "daemon.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<Message>();
    Daemon daemon;
    return a.exec();
}
