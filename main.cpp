#include "browserwindow.h"
#include <QEventLoop>
#include "nodethread.h"
#include <QtWebEngine>
#include <QTextCodec>
#include <iostream>


#include <QDebug>
#include <QApplication>
#include <QMutex>
#include <iostream>

void messageOutputFilter(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // filter messages
    if( type==QtWarningMsg && (msg.indexOf("of object 'BrowserWindow' has no notify signal and is not constant")>=0
            || msg.indexOf("Remote debugging server started successfully. Try pointing a Chromium-based browser to")>=0
    ))
        return ;

    QByteArray localMsg = msg.toLocal8Bit();

    QString strMsg("");
    switch(type)
    {
    case QtDebugMsg:
        strMsg = QString("Debug:");
        break;
    case QtWarningMsg:
        strMsg = QString("Warning:");
        break;
    case QtCriticalMsg:
        strMsg = QString("Critical:");
        break;
    case QtFatalMsg:
        strMsg = QString("Fatal:");
        break;
    }

    // 输出信息
    std::cout << localMsg.toStdString().c_str() << std::endl << std::flush ;
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageOutputFilter) ;
    srand((unsigned)time(NULL));

    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING",QString("%1").arg((rand() % 50000)+10000).toStdString().c_str());
    QtWebEngine::initialize();

    QString sdkPath ;

    QStringList nodeArgv ;
    for(int i=0; i<argc; i++) {
        if( strcmp(argv[i],"--sdk")==0 ) {
            if( i<argc-1 ){

                qd(">>" << sdkPath.toStdString().c_str())
                sdkPath = argv[++i] ;
            }
            continue ;
        }
        nodeArgv << argv[i] ;
    }

    NodeThread node(nodeArgv, sdkPath) ;
    node.start();

    int code = a.exec();

    QEventLoop loop;
    QObject::connect(&node,  SIGNAL(finished()), &loop, SLOT(quit()) );
    loop.exec();

    return 0 ;
}
