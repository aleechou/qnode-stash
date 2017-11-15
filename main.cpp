#include "browserwindow.h"
#include <QApplication>
#include <QThread>
#include "nodethread.h"
#include <qdebug.h>

char * dot = "." ;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString sdkPath ;

    QStringList nodeArgv ;
    for(int i=0; i<argc; i++) {
        if( strcmp(argv[i],"--sdk")==0 ) {
            if( i<argc-1 ){
                sdkPath = argv[++i] ;
            }
            continue ;
        }
        nodeArgv << argv[i] ;
    }
    nodeArgv << "." ;

    NodeThread node(nodeArgv, sdkPath) ;
    node.start();


    return a.exec();

}
