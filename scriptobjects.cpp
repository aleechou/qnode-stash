#include "scriptobjects.h"
#include <QMap>
#include <QMetaObject>
#include <QMetaMethod>
#include <QThread>
#include "common.h"

ScriptObjects * ScriptObjects::global = nullptr ;
static unsigned int ScriptObjectAssignedId = 0 ;
static QMap<unsigned int, QObject *> poolScriptObjects ;

unsigned int ScriptObjects::registerScriptObject(QObject * parent) {
    unsigned int id = ScriptObjectAssignedId ++ ;
    poolScriptObjects.insert(id, parent);
    return id ;
}

QVariant ScriptObjects::callMethod(unsigned int objId, const QString & method, const QVariantList & args) {
    QObject * object = ScriptObjects::queryScriptObjectById(objId) ;
    if(!object) {
        qDebug() << "unknow script object id:" << objId ;
        return QVariant();
    }
    return callMethod(object, method, args) ;
}

QVariant ScriptObjects::callMethod(QObject * object, const QString & method, const QVariantList & args) {
    int index = object->metaObject()->indexOfMethod(method.toStdString().c_str());
    if(index<0) {
        qDebug() << "unknow method" << method << "for class" << object->metaObject()->className() ;
        return QVariant();
    }
    QMetaMethod metaMethod = object->metaObject()->method(index);
    return ScriptObjects::call(object, metaMethod,args) ;
}

void ScriptObjects::invokeMethod(unsigned int objId, const QString & method, const QVariantList & args, QObject * from, unsigned int reqId) {
    QObject * object = ScriptObjects::queryScriptObjectById(objId) ;
    if(!object) {
        qDebug() << "unknow script object id:" << objId ;
        return ;
    }
    invokeMethod(object, method, args, from, reqId) ;
}

void ScriptObjects::invokeMethod(QObject * object, const QString & method, const QVariantList & args, QObject * from, unsigned int reqId) {
    QObject * anchor = new QObject() ;
    anchor->moveToThread(object->thread());

    QObject::connect(anchor, &QObject::destroyed, [object, method, args, from, reqId](){

        int index = object->metaObject()->indexOfMethod(method.toStdString().c_str());
        if(index<0) {
            qDebug() << "unknow method" << method << "for class" << object->metaObject()->className() ;
            return ;
        }
        QMetaMethod metaMethod = object->metaObject()->method(index);

        QVariant ret = ScriptObjects::call(object, metaMethod,args) ;

        if(from!=nullptr && ret.isValid()) {
            QMetaObject::invokeMethod(from, "invokeReturn", Q_ARG(unsigned int, reqId), Q_ARG(QVariant, ret)) ;
        }

    }) ;
    anchor->deleteLater();
}



QVariant ScriptObjects::call(QObject* object, QMetaMethod metaMethod, const QVariantList & args)
{
    // Convert the arguments

    QVariantList converted;

    // We need enough arguments to perform the conversion.

    QList<QByteArray> methodTypes = metaMethod.parameterTypes();
    if (methodTypes.size() < args.size()) {
        qWarning() << "Insufficient arguments to call" << metaMethod.methodSignature();
        return QVariant();
    }

    for (int i = 0; i < methodTypes.size(); i++) {
        const QVariant& arg = args.at(i);

        QByteArray methodTypeName = methodTypes.at(i);
        QByteArray argTypeName = arg.typeName();

        QVariant::Type methodType = QVariant::nameToType(methodTypeName);
        QVariant::Type argType = arg.type();

        QVariant copy = QVariant(arg);

        // If the types are not the same, attempt a conversion. If it
        // fails, we cannot proceed.

        if (copy.type() != methodType) {
            if (copy.canConvert(methodType)) {
                if (!copy.convert(methodType)) {
                    qWarning() << "Cannot convert" << argTypeName
                               << "to" << methodTypeName;
                    return QVariant();
                }
            }
        }

        converted << copy;
    }

    QList<QGenericArgument> arguments;

    for (int i = 0; i < converted.size(); i++) {

        // Notice that we have to take a reference to the argument, else
        // we'd be pointing to a copy that will be destroyed when this
        // loop exits.

        QVariant& argument = converted[i];

        // A const_cast is needed because calling data() would detach
        // the QVariant.

        QGenericArgument genericArgument(
            QMetaType::typeName(argument.userType()),
            const_cast<void*>(argument.constData())
        );

        arguments << genericArgument;
    }

    if( strcmp(metaMethod.typeName(),"void")==0 ) {
        bool ok = metaMethod.invoke(
            object,
            Qt::DirectConnection,
            arguments.value(0),
            arguments.value(1),
            arguments.value(2),
            arguments.value(3),
            arguments.value(4),
            arguments.value(5),
            arguments.value(6),
            arguments.value(7),
            arguments.value(8),
            arguments.value(9)
        );

        if (!ok) {
            qWarning() << "Calling" << metaMethod.methodSignature() << "failed.";
        }
        return QVariant();
    }
    else {

        QVariant returnValue(QMetaType::type(metaMethod.typeName()),
            static_cast<void*>(NULL));

        QGenericReturnArgument returnArgument(
            metaMethod.typeName(),
            const_cast<void*>(returnValue.constData())
        );

        // Perform the call
        bool ok = metaMethod.invoke(
            object,
            Qt::DirectConnection,
            returnArgument,
            arguments.value(0),
            arguments.value(1),
            arguments.value(2),
            arguments.value(3),
            arguments.value(4),
            arguments.value(5),
            arguments.value(6),
            arguments.value(7),
            arguments.value(8),
            arguments.value(9)
        );
        if (!ok) {
            qWarning() << "Calling" << metaMethod.methodSignature() << "failed.";
            return QVariant();
        } else {
            return returnValue;
        }
    }
}

QObject * ScriptObjects::queryScriptObjectById(unsigned int id) {
    if( !poolScriptObjects.contains(id) ){
        return nullptr ;
    }
    return poolScriptObjects.value(id) ;
}


ScriptObjects * ScriptObjects::singleton() {
    if(ScriptObjects::global==nullptr) {
        ScriptObjects::global = new ScriptObjects() ;
    }
    return ScriptObjects::global ;
}

QString ScriptObjects::className(unsigned int objId) {
    QObject * object = ScriptObjects::queryScriptObjectById(objId) ;
    if(nullptr==object) {
        return QString("unknow object id: %1").arg(objId);
    }
    return object->metaObject()->className() ;
}

QString ScriptObjects::methodList(unsigned int objId) {
    QObject * object = ScriptObjects::queryScriptObjectById(objId) ;
    if(nullptr==object) {
        return QString("unknow object id: %1").arg(objId);
    }
    const QMetaObject* metaObject = object->metaObject();

    QString output = "[\r\n" ;

    for(int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod metaMethod = metaObject->method(i) ;
        output+= "  {\r\n" ;
        output+= "    name:\"" + QString(metaMethod.name()) + "\",\r\n" ;
        output+= "    returnType:\"" + QString(metaMethod.typeName()) + "\",\r\n" ;

        output+= "    params:[\r\n" ;
        QList<QByteArray> parameterNames = metaMethod.parameterNames() ;
        QList<QByteArray> parameterTypes = metaMethod.parameterTypes() ;
        for(int p=0;p<metaMethod.parameterCount();p++){
            output+= "      {\r\n" ;
            output+= "        name: \"" +QString(parameterNames[p])+ "\",\r\n" ;
            output+= "        type: \"" +QString(parameterTypes[p])+ "\",\r\n" ;
            output+= "      },\r\n" ;

        }
        output+= "    ],\r\n" ;

        output+= "    signature:\"" + QString(metaMethod.methodSignature()) + "\",\r\n" ;
        output+= "  },\r\n" ;
    }

    return output + "]" ;
}
