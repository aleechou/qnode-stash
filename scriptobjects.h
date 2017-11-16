#ifndef ScriptObject_H
#define ScriptObject_H

#include <QObject>
#include <QCoreApplication>
#include "common.h"

class ScriptObjects : public QObject
{
    Q_OBJECT
public:
    ScriptObjects() {
        _id = ScriptObjects::registerScriptObject(this) ;
        moveToThread(QCoreApplication::instance()->thread());
    }

    unsigned int id() {
        return _id ;
    }

    static unsigned int registerScriptObject(QObject *parent) ;
    static QVariant callMethod(unsigned int id, const QString &, const QVariantList &) ;
    static QVariant callMethod(QObject *, const QString &, const QVariantList &) ;
    static void invokeMethod(unsigned int id, const QString &, const QVariantList &, QObject * from=nullptr,unsigned int reqId=0) ;
    static void invokeMethod(QObject *, const QString &, const QVariantList &, QObject * from=nullptr,unsigned int reqId=0) ;
    static QObject * queryScriptObjectById(unsigned int id) ;

    static QVariant call(QObject* object, QMetaMethod metaMethod, const QVariantList & args) ;

    static ScriptObjects * singleton() ;

    Q_INVOKABLE QString className(unsigned int) ;
    Q_INVOKABLE QString methodList(unsigned int) ;

private:
    static ScriptObjects * global ;
    unsigned int _id ;
};



#endif // ScriptObject_H
