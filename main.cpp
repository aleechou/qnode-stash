#include "browserwindow.h"
#include <QApplication>
#include <QThread>
#include "nodethread.h"
#include <qdebug.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "main()" << QThread::currentThread() ;

    NodeThread node(argc, argv) ;
    node.start();



    return a.exec();

}
