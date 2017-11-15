#include "ui_browserwindow.h"
#include <QUrl>
#include <QCoreApplication>
#include "browserwindow.h"
#include "common.h"
#include <QMetaObject>
#include <QJsonValue>

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
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}

void BrowserWindow::load(const QString & url) {
    ui->browser->load(QUrl(url)) ;
}
