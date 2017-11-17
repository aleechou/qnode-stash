#include "nodethread.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QJsonDocument>
#include "browserwindow.h"
#include "nodeapi.h"
#include "common.h"
#include <QFile>


NodeThread::NodeThread(const QStringList & argv, const QString & sdkPath, QObject *parent)
    : argv(argv)
    , QThread(parent)
{
    objectId = ScriptObjects::registerScriptObject(this) ;

    // 计算参数长度
    int nodeArgvSize = 0 ;
    foreach(QString arg, argv) {
        nodeArgvSize+= arg.length()+1 ;
    }

    // libuv 在unix平台上要求 argv 内的数据是一个连续的内存
    argvData = new char[nodeArgvSize] ;
    argvArray = new char * [argv.length()] ; ;

    char * argvIdx = argvData ;
    int i = 0 ;
    foreach(QString arg, argv) {
        strcpy(argvIdx, arg.toUtf8().data()) ;
        argvArray[i++] = argvIdx ;
        argvIdx+= arg.length() + 1 ;
    }



    if(!sdkPath.isEmpty())
        this->sdkPath = sdkPath ;
    moveToThread(this);
}

void NodeThread::run() {

    node::StartEx(argv.length(), argvArray, NodeThread::beforeloop);

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


void NodeThread::beforeloop(v8::Isolate * isolate, void * loop){

    NodeThread * thread = (NodeThread*)QThread::currentThread() ;
    thread->isolate = isolate ;

    v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();

    // qnode api
    DefineMethod("$qnode_api_invoke", NodeThread, jsInvoke);
    DefineMethod("$qnode_api_call", NodeThread, jsCall);
    DefineMethod("$qnode_api_on", NodeThread, jsOn);

    global->Set(v8string("$qnode_api_thread"), v8int32(thread->objectId)) ;
    global->Set(v8string("$qnode_api_browser_window_creator"), v8int32(BrowserWindowCreator::singleton()->id())) ;
    global->Set(v8string("$qnode_api_script_objects"), v8int32(ScriptObjects::singleton()->id())) ;

    // load qnode sdk
    if(!thread->sdkPath.isEmpty()) {
        thread->requireScript(thread->sdkPath) ;
    }

    // process qt event loop
    thread->uvidler = new uv_idle_t;

    uv_idle_init((uv_loop_t*)loop, thread->uvidler);

    uv_idle_start(thread->uvidler, [](uv_idle_t*){
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

    file.close();

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
            return ;                                                        \
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


void NodeThread::jsOn(const v8::FunctionCallbackInfo<v8::Value> & args){

    NodeThread* thread = (NodeThread*)QThread::currentThread() ;
    v8::Isolate * isolate = args.GetIsolate() ;

    int objId = args[0]->ToInt32()->Value() ;
    QObject * object = ScriptObjects::queryScriptObjectById(objId) ;
    if(nullptr==object){
        qDebug() << "unknow script object" << objId ;
        args.GetReturnValue().Set(v8::Boolean::New(isolate, false)) ;
        return ;
    }

    const QString signal = ToQString(args[1]) ;
    int connId = thread->invokeAnotherThreadReqId ++ ;

    const QMetaObject * metaObj = object->metaObject() ;
    int sigindex = metaObj->indexOfSignal(signal.toStdString().c_str()) ;
    if(sigindex<0) {
        qDebug() << "unknow signal" << signal << "of class" << metaObj->className() ;
        args.GetReturnValue().Set(v8::Boolean::New(isolate, false)) ;
        return ;
    }

    DynamicConnectionReceiver * receiver = new DynamicConnectionReceiver(connId, thread) ;
    QMetaObject::connect(object, sigindex, receiver, receiver->metaObject()->indexOfSlot("slot()")) ;

    args.GetReturnValue().Set(v8int32(connId)) ;
}

void DynamicConnectionReceiver::slot(){
    sender() ;
    QString script = QString("$qnode_api_emit(%1)").arg(connId) ;
    v8::Isolate * isolate = from->isolate ;
    v8::HandleScope scope(from->isolate);
    v8::Script::Compile ( v8string(script.toStdString().c_str()) )->Run();
}
