#include "ui_browserwindow.h"
#include <QUrl>
#include <QCoreApplication>
#include "browserwindow.h"
#include "common.h"
#include <QMetaObject>
#include <QFile>
#include <QDesktopServices>
#include <QWebChannel>
#include <QWebEnginePage>

BrowserWindowCreator * BrowserWindowCreator::global=nullptr ;

BrowserWindowCreator::BrowserWindowCreator() {
    moveToThread(QCoreApplication::instance()->thread()) ;
    _id = ScriptObjects::registerScriptObject(this) ;
}
unsigned int BrowserWindowCreator::createBrowserWindow(unsigned int parentId){
    BrowserWindow * window = new BrowserWindow(parentId);
    return window->id() ;
}


BrowserWindowCreator * BrowserWindowCreator::singleton() {
    if(BrowserWindowCreator::global==nullptr) {
        BrowserWindowCreator::global = new BrowserWindowCreator() ;
    }
    return BrowserWindowCreator::global ;
}


BrowserWindow::BrowserWindow(unsigned int parentObjectId, QWidget *parent) :
    QWidget(parent),
    parentObjectId(parentObjectId),
    ui(new Ui::BrowserWindow)
{
    ui->setupUi(this);
    objectId = ScriptObjects::registerScriptObject(this) ;

    // web channel
    QWebChannel * channel = new QWebChannel(this);
    channel->registerObject("$window", this);
    channel->registerObject("$qnodeapi_script_objects", ScriptObjects::singleton());
    channel->registerObject("$qnodeapi_browser_window_creator", BrowserWindowCreator::singleton());
    ui->browser->page()->setWebChannel(channel);

    QObject::connect(ui->browser->page(), &QWebEnginePage::loadFinished,[this](bool ok){

        if(!ok) {
            emit this->ready(ok);
        }

        QWebEnginePage * page = ui->browser->page() ;

        page->runJavaScript(apiFs.readFile(":/qtwebchannel/qwebchannel.js")) ;
        page->runJavaScript(apiFs.readFile(":/sdk/webkit/api.js")) ;

        QString boot = QString(
                    "qnode.api.threadId = %1;\r\n"
                    "qnode.api.parentThreadId = %2;\r\n"
                    "$qnodeapi_console_port = %3;\r\n"
                    "new QWebChannel(qt.webChannelTransport, function(channel) { ;\r\n"
                    "    for (var name in channel.objects)\r\n"
                    "        window[name] = channel.objects[name] ;\r\n"
                    "    qnode.window = $window ;\r\n"
                    "    $window.onLoaded() ;\r\n"
                    "})\r\n"
                )
                    .arg(this->objectId)
                    .arg(this->parentObjectId)
                    .arg(QString(qgetenv("QTWEBENGINE_REMOTE_DEBUGGING"))) ;
        page->runJavaScript(boot);
    }) ;
}

void BrowserWindow::onLoaded() {
    QWebEnginePage * page = ui->browser->page() ;
    page->runJavaScript(apiFs.readFile(":/sdk/webkit/eventemitter.js")) ;
    page->runJavaScript(apiFs.readFile(":/sdk/webkit/window.js")) ;
    page->runJavaScript(apiFs.readFile(":/sdk/common/api.run.js")) ;
    page->runJavaScript(apiFs.readFile(":/sdk/common/bridge.js")) ;

    emit this->ready(true) ;
}

void BrowserWindow::runScriptInThread(unsigned int threadId, const QString & script) {
    QObject * parentThread = ScriptObjects::queryScriptObjectById(threadId) ;
    if(!parentThread) {
        return ;
    }
    bool ok = QMetaObject::invokeMethod(parentThread, "runScript", Qt::QueuedConnection, Q_ARG(QString, script)) ;
    if(!ok) {
        qCritical() << "can not QMetaObject::invokeMethod() runScript(QString) , for" << parentThread  ;
    }
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}

void BrowserWindow::load(const QString & url) {
    ui->browser->load(QUrl(url)) ;
}

void BrowserWindow::loadScript(const QString & url) {
    ui->browser->page()->runJavaScript(QString("var script = document.createElement('script'); script.src='%1'; document.body.appendChild(script)").arg(url)) ;
}


void BrowserWindow::runScript(const QString & script) {
    ui->browser->page()->runJavaScript(script) ;
}

bool BWApiFs::exists(const QString & filepath) {
    return false ;
}

QByteArray BWApiFs::readFile(const QString & filepath) {
    QFile file(filepath) ;
    if(!file.open(QFile::ReadOnly)){
        qDebug() << "file not exits," << filepath ;
        return QByteArray() ;
    }
    QByteArray content = file.readAll() ;
    file.close() ;
    return content ;
}
