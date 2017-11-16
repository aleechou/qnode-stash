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
unsigned int BrowserWindowCreator::createBrowserWindow(){
    BrowserWindow * window = new BrowserWindow();
    return window->id() ;
}


BrowserWindowCreator * BrowserWindowCreator::singleton() {
    if(BrowserWindowCreator::global==nullptr) {
        BrowserWindowCreator::global = new BrowserWindowCreator() ;
    }
    return BrowserWindowCreator::global ;
}


BrowserWindow::BrowserWindow(QWidget *parent) :
    QWidget(parent),
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

        page->runJavaScript("new QWebChannel(qt.webChannelTransport, function(channel) {;"
        "    for (var name in channel.objects)"
        "        window[name] = channel.objects[name];"
        "})") ;

        emit this->loaded(ok) ;
    }) ;
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}

void BrowserWindow::load(const QString & url) {
    ui->browser->load(QUrl(url)) ;
}

void BrowserWindow::openConsole() {
    QDesktopServices::openUrl ( QUrl("http://127.0.0.1:17135") ) ;
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
