#include "nodethread.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QJsonDocument>
#include "browserwindow.h"
#include "nodeapi.h"
#include "common.h"


NodeThread::NodeThread(int argc, char **argv, QObject *parent)
    : argc(argc)
    , argv(argv)
    , QThread(parent)
{
    moveToThread(this);
}

void NodeThread::run() {

    uvidler = new uv_idle_t;

    node::StartEx(argc, argv, NodeThread::beforeloop);

    uv_idle_stop(uvidler);
    delete uvidler;

}


void NodeThread::invokeReturn(unsigned int invokeId, const QVariant & value) {

//    qDebug() << "invokeReturn()" << invokeId << value << QThread::currentThread() ;

    QString stringRetValue ;
    if( value.type() == QVariant::String ){
        stringRetValue = "`" + value.toString() + "`" ;
    }
    else {
        stringRetValue = value.toString() ;
    }

    QString script = QString("__$qnode_invoke_return(%1, %2)").arg(invokeId).arg(stringRetValue) ;
    v8::HandleScope scope(isolate);
    v8::Script::Compile ( v8string(script.toStdString().c_str()) )->Run();
}

void NodeThread::onRunScript(const QString & script, unsigned int reqId){
//    qDebug() << "onRunScript()" << QThread::currentThread() ;
    if( !isolate ){
        return ;
    }
    v8::HandleScope scope(isolate);
    v8::Script::Compile ( v8string(script.toStdString().c_str()) )->Run();
}

#include <iostream>

void NodeThread::beforeloop(v8::Isolate * isolate, void * loop){

    //
    v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();

    DefineMethod("create_browser_window", NodeThread, jsCreateBrowserWindow);
    DefineMethod("run_script_in_thread", NodeThread, jsRunScript);
    DefineMethod("invoke", NodeThread, jsInvoke);

    global->Set(v8string("__$qnode_objectid_browser_window_creator"), v8int32(BrowserWindowCreator::singleton()->id())) ;
    global->Set(v8string("__$qnode_objectid_script_objects"), v8int32(ScriptObjects::singleton()->id())) ;
    NodeThread * thread = (NodeThread*)QThread::currentThread() ;

    thread->isolate = isolate ;

    uv_idle_init((uv_loop_t*)loop, thread->uvidler);

    uv_idle_start(thread->uvidler, [](uv_idle_t*, uv_idle_cb){
        NodeThread * thread = (NodeThread*)QThread::currentThread() ;
        thread->eventDispatcher()->processEvents(QEventLoop::EventLoopExec) ;
    }) ;

}



void NodeThread::jsInvoke(const v8::FunctionCallbackInfo<v8::Value> & args){

    NodeThread* thread = (NodeThread*)QThread::currentThread() ;
    unsigned int reqId = thread->invokeAnotherThreadReqId ++ ;

    int objId = args[0]->ToInt32()->Value() ;
    const QString method = ToQString(args[1]) ;

    QVariantList invokeArgs ;
    for(int i=2; i<args.Length(); i++){
        if( args[i]->IsInt32() ) {
            invokeArgs.append(QVariant(args[i]->ToInt32()->Value())) ;
        }
        else if( args[i]->IsBoolean() ){
            invokeArgs.append(QVariant(args[i]->ToBoolean()->Value())) ;
        }
        else if( args[i]->IsString() ){
            invokeArgs.append(QVariant( ToQString(args[i]) )) ;
        }
        else {
            qDebug() << "unsuported args type" << "" ;
            return ;
        }
    }

    ScriptObjects::invokeMethod(objId, method, invokeArgs, (QObject *)thread, reqId) ;

    v8::Isolate * isolate = args.GetIsolate() ;
    args.GetReturnValue().Set(v8int32(reqId)) ;
}
void NodeThread::jsCreateBrowserWindow(const v8::FunctionCallbackInfo<v8::Value> & args){
    NodeThread* thread = (NodeThread*)QThread::currentThread() ;
    unsigned int reqId = thread->invokeAnotherThreadReqId ++ ;

    QMetaObject::invokeMethod(
                (QObject *)BrowserWindowCreator::singleton()
                , "createBrowserWindow"
                , Q_ARG(QObject *, (QObject *)thread)
                , Q_ARG(unsigned int, reqId)
    ) ;

}


void NodeThread::jsRunScript(const v8::FunctionCallbackInfo<v8::Value> & args){

}
