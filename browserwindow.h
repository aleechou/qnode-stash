#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QWidget>
#include <QMap>
#include <QThread>
#include "scriptobjects.h"

class BrowserWindowCreator: public QObject
{
    Q_OBJECT

public:
    BrowserWindowCreator() ;

    Q_INVOKABLE unsigned int createBrowserWindow() ;

    static BrowserWindowCreator * singleton() ;

    unsigned int id() {
        return _id ;
    }

private:
    static BrowserWindowCreator * global ;
    unsigned int _id ;
};



namespace Ui {
class BrowserWindow;
}

class BrowserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = 0);
    ~BrowserWindow();

    unsigned int id() {return objectId ;}

    Q_INVOKABLE void load(const QString &) ;

signals:
    void runScript() ;

private:
    Ui::BrowserWindow *ui;

    unsigned int objectId;
};



#endif // BROWSERWINDOW_H
