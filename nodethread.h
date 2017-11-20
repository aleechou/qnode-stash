#ifndef NODETHREAD_H
#define NODETHREAD_H

#include <QThread>
#include <v8.h>
#include <node.h>
#include <uv.h>
#include "scriptobjects.h"
#include <QVariant>

class DynamicConnectionReceiver ;


class NodeThread : public QThread
{
    Q_OBJECT
public:
    explicit NodeThread(const QStringList &, const QString & sdk=QString(),QObject *parent = nullptr);

    static void beforeloop(v8::Isolate *, void *) ;

    static void jsReadInnerModule(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsInnerModuleExists(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsInvoke(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsCall(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsOn(const v8::FunctionCallbackInfo<v8::Value> & args) ;

    Q_INVOKABLE void invokeReturn(unsigned int invokeId, const QVariant &) ;
    Q_INVOKABLE void runScript(const QString & script) ;

    friend class DynamicConnectionReceiver ;
protected:
    void run() ;

private:
    unsigned int objectId ;

    QStringList argv ;
    char ** argvArray = nullptr ;
    char * argvData = nullptr ;
    QString sdk = "qrc://sdk/" ;
    v8::Isolate * isolate = nullptr ;

    uv_idle_t * uvidler = nullptr ;

    unsigned int invokeAnotherThreadReqId = 0 ;

    Q_INVOKABLE bool requireScript(const QString &) ;
};


class DynamicConnectionReceiver: public QObject {
    Q_OBJECT
public:
    explicit DynamicConnectionReceiver(unsigned int connId, NodeThread * from):
        connId(connId), from(from), QObject(from) {}
public slots:
    void slot() ;
private:
    unsigned int connId ;
    NodeThread * from ;
};
unsigned int registerScriptThread(QObject * ) ;
QObject * queryScriptThreadById(unsigned int) ;

#endif // NODETHREAD_H
