#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QWidget>
#include <QMap>
#include <QJsonValue>
#include <QJsonArray>
#include "scriptobjects.h"

class BrowserWindowCreator: public QObject
{
    Q_OBJECT

public:
    BrowserWindowCreator() ;

    Q_INVOKABLE unsigned int createBrowserWindow(unsigned int parentId) ;

    static BrowserWindowCreator * singleton() ;

    unsigned int id() {
        return _id ;
    }

private:
    static BrowserWindowCreator * global ;
    unsigned int _id ;
};




class BWApiFs: public QObject {
Q_OBJECT
public:
    BWApiFs(QObject *parent = 0): QObject(parent) {}

    Q_INVOKABLE bool exists(const QString & filepath) ;
    Q_INVOKABLE QByteArray readFile(const QString & filepath) ;
};

namespace Ui {
class BrowserWindow;
}

class BrowserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserWindow(unsigned int parentObjectId ,QWidget *parent = 0);
    ~BrowserWindow();

    Q_INVOKABLE unsigned int parentNodeThreadId() {
        return this->parentObjectId ;
    }
    unsigned int id() {return objectId ;}

    Q_INVOKABLE void loadScript(const QString & url) ;
    Q_INVOKABLE void runScript(const QString & script) ;
    Q_INVOKABLE void load(const QString & url) ;
    Q_INVOKABLE void onLoaded() ;

    Q_INVOKABLE void runScriptInThread(unsigned int, const QString &) ;

signals:
    void ready(bool) ;


private:
    Ui::BrowserWindow *ui;
    BWApiFs apiFs ;

    unsigned int objectId;
    unsigned int parentObjectId ;
};



#endif // BROWSERWINDOW_H
