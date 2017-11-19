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
    ui->browser->page()->setWebChannel(channel);

    QObject::connect(ui->browser->page(), &QWebEnginePage::loadFinished,[this](bool ok){

        QWebEnginePage * page = ui->browser->page() ;

        page->runJavaScript(apiFs.readFile(":/qtwebchannel/qwebchannel.js")) ;
        page->runJavaScript(apiFs.readFile(":/sdk/webkit/require.js")) ;

        page->runJavaScript(QString("new QWebChannel(qt.webChannelTransport, function(channel) {;"
        "    for (var name in channel.objects)"
        "        window[name] = channel.objects[name];"
        "    $window.parentNodeThreadId = %1;"
        "})").arg(this->parentNodeThreadId())) ;

        emit this->ready(ok) ;
    }) ;
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}

void BrowserWindow::load(const QString & url) {
    ui->browser->load(QUrl(url)) ;
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
