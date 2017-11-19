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

    // 设置输出信息格式
    QString strMessage = QString("[@%2] %1")
            .arg(localMsg.constData()).arg(context.line);

    // 输出信息
    std::cout << strMessage.toStdString().c_str() << std::endl << std::flush ;
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageOutputFilter) ;

    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING","17135");
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
