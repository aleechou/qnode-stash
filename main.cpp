#include "browserwindow.h"
#include <QApplication>
#include <QEventLoop>
#include "nodethread.h"
#include <qdebug.h>
#include <QtWebEngine>
#include <QTextCodec>

void messageOutputFilter(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        if( msg.indexOf("of object 'BrowserWindow' has no notify signal and is not constant")<0
                && msg.indexOf("Remote debugging server started successfully. Try pointing a Chromium-based browser to")<0
        )
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageOutputFilter);

    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING","17135");
    QtWebEngine::initialize();

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

    int code = a.exec();

    QEventLoop loop;
    QObject::connect(&node,  SIGNAL(finished()), &loop, SLOT(quit()) );
    loop.exec();

    return 0 ;
}
