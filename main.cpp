#include "browserwindow.h"
#include <QApplication>
#include <QThread>
#include "nodethread.h"
#include <qdebug.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString sdkPath ;

    char * nodeArgv[argc] ;
    int nodeArgc = 0 ;
    for(int i=0; i<argc; i++) {
        if( strcmp(argv[i],"--sdk")==0 ) {
            if( i<argc-1 ){
                sdkPath = argv[++i] ;
            }
            continue ;
        }

        nodeArgv[nodeArgc++] = argv[i] ;
    }

//    qDebug() << "main()" << QThread::currentThread() ;

    NodeThread node(nodeArgc, nodeArgv, sdkPath) ;
    node.start();


    return a.exec();

}
