#include "window.h"
#include <QApplication>
#include "gui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gui b;
    b.showwindow();
    b.addroom("gking","3/8");
    b.addroom("gking301","3/80");
    b.setup("123","654");
    b.addplayer("gking","???");

    return a.exec();
}
