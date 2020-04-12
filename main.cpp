#include "mainwindow.h"
#include "Server/myserver.h"
#include <QApplication>
#include "Client/myclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    MyServer myServer(10000);
    myServer.show();
    MyClient myClient("localhost", 10000);
    myClient.show();
    return a.exec();
}
