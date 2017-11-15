#ifndef NODETHREAD_H
#define NODETHREAD_H

#include <QThread>
#include <v8.h>
#include <node.h>
#include <uv.h>
#include "scriptobjects.h"
#include <QVariant>

class NodeThread : public QThread
{
    Q_OBJECT
public:
    explicit NodeThread(int argc, char *argv[], QObject *parent = nullptr);

    static void beforeloop(v8::Isolate *, void *) ;

    static void NodeThread::nodeInitialize(v8::Local<v8::Object> exports) ;
    static void jsInvoke(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsCreateBrowserWindow(const v8::FunctionCallbackInfo<v8::Value> & args) ;
    static void jsRunScript(const v8::FunctionCallbackInfo<v8::Value> & args) ;



    Q_INVOKABLE void invokeReturn(unsigned int invokeId, const QVariant &) ;
protected:
    void run() ;

private:
    char ** argv ;
    int argc ;
    v8::Isolate * isolate = nullptr ;

    uv_idle_t * uvidler = nullptr ;

    unsigned int invokeAnotherThreadReqId = 0 ;



public slots:
     void onRunScript(const QString &, unsigned int reqId) ;
};

unsigned int registerScriptThread(QObject * ) ;
QObject * queryScriptThreadById(unsigned int) ;

#endif // NODETHREAD_H
