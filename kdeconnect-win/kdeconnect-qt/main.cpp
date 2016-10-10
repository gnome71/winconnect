#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	a.setOrganizationName("KdeConnect-Win");
	a.setOrganizationDomain("kdeconnectwin.win");
	a.setApplicationName("KdeConnect-Win");
    MainWindow w;
    w.show();

    return a.exec();
}
