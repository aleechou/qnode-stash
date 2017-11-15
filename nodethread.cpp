#include "nodethread.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QJsonDocument>
#include "browserwindow.h"
#include "nodeapi.h"
#include "common.h"


NodeThread::NodeThread(int argc, char **argv, const QString & sdkPath, QObject *parent)
    : argc(argc)
    , argv(argv)
    , QThread(parent)
{
    if(!sdkPath.isEmpty())
        this->sdkPath = sdkPath ;
    moveToThread(this);
}

void NodeThread::run() {

    node::StartEx(argc, argv, NodeThread::beforeloop);

    if(uvidler) {
        uv_idle_stop(uvidler);
        delete uvidler;
    }

}

inline QString variant_qt_to_js(const QVariant & value){
    if( value.type() == QVariant::String ){
        return "`" + value.toString() + "`" ;
    }
    else {
        return value.toString() ;
    }
}

void NodeThread::invokeReturn(unsigned int invokeId, const QVariant & value) {
    QString stringRetValue = variant_qt_to_js(value) ;
    QString script = QString("$qnode_api_invoke_return(%1, %2)").arg(invokeId).arg(stringRetValue) ;
    v8::HandleScope scope(isolate);
    v8::Script::Compile ( v8string(script.toStdString().c_str()) )->Run();
}

#include <QFile>

void NodeThread::beforeloop(v8::Isolate * isolate, void * loop){

    NodeThread * thread = (NodeThread*)QThread::currentThread() ;
    thread->isolate = isolate ;

    v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();

    // qnode api
    DefineMethod("$qnode_api_invoke", NodeThread, jsInvoke);
    DefineMethod("$qnode_api_call", NodeThread, jsCall);

    global->Set(v8string("$qnode_api_browser_window_creator"), v8int32(BrowserWindowCreator::singleton()->id())) ;
    global->Set(v8string("$qnode_api_script_objects"), v8int32(ScriptObjects::singleton()->id())) ;

    // load qnode sdk
    qd << thread->sdkPath ;
    if(!thread->sdkPath.isEmpty()) {
        thread->requireScript(thread->sdkPath) ;
    }

    // process qt event loop
    thread->uvidler = new uv_idle_t;

    uv_idle_init((uv_loop_t*)loop, thread->uvidler);

    uv_idle_start(thread->uvidler, [](uv_idle_t*, uv_idle_cb){
        NodeThread * thread = (NodeThread*)QThread::currentThread() ;
        thread->eventDispatcher()->processEvents(QEventLoop::EventLoopExec) ;
    }) ;

}


void NodeThread::runScript(const QString & script){
    if( !isolate ){
        qDebug() << "can not call runScript(), isolate is null" ;
        return ;
    }
    v8::HandleScope scope(isolate);
    v8::Script::Compile ( v8string(script.toStdString().c_str()) )->Run();
}

bool NodeThread::requireScript(const QString & path){
    QFile file(path) ;
    if(!file.open(QFile::ReadOnly)) {
        qd << "can not open file" << path ;
        return false ;
    }

    this->runScript(file.readAll());

    return true ;
}


#define toQtArgs \
    QVariantList invokeArgs ;                                               \
    for(int i=2; i<args.Length(); i++){                                     \
        if( args[i]->IsInt32() ) {                                          \
            invokeArgs.append(QVariant(args[i]->ToInt32()->Value())) ;      \
        }                                                                   \
        else if( args[i]->IsBoolean() ){                                    \
            invokeArgs.append(QVariant(args[i]->ToBoolean()->Value())) ;    \
        }                                                                   \
        else if( args[i]->IsString() ){                                     \
            invokeArgs.append(QVariant( ToQString(args[i]) )) ;             \
        }                                                                   \
        else {                                                              \
            qDebug() << "unsuported args type: " << ToQString(args[i]) ;    \
            return QVariant() ;                                             \
        }                                                                   \
    }

void NodeThread::jsInvoke(const v8::FunctionCallbackInfo<v8::Value> & args){

    NodeThread* thread = (NodeThread*)QThread::currentThread() ;
    unsigned int reqId = thread->invokeAnotherThreadReqId ++ ;

    int objId = args[0]->ToInt32()->Value() ;
    const QString method = ToQString(args[1]) ;

    toQtArgs

    ScriptObjects::invokeMethod(objId, method, invokeArgs, (QObject *)thread, reqId) ;

    v8::Isolate * isolate = args.GetIsolate() ;
    args.GetReturnValue().Set(v8int32(reqId)) ;
}

void NodeThread::jsCall(const v8::FunctionCallbackInfo<v8::Value> & args){

    NodeThread* thread = (NodeThread*)QThread::currentThread() ;

    int objId = args[0]->ToInt32()->Value() ;
    const QString method = ToQString(args[1]) ;

    toQtArgs

    QVariant ret = ScriptObjects::callMethod(objId, method, invokeArgs) ;

    v8::Isolate * isolate = args.GetIsolate() ;
    args.GetReturnValue().Set( v8string(ret.toString().toStdString().c_str()) ) ;
}

