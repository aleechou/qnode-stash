#ifndef NODEAPI_H
#define NODEAPI_H

#include <v8.h>
#include <node.h>
#include <QObject>


#define Throw(msg)                                                              \
		args.GetIsolate()->ThrowException(v8::Exception::Error(                 \
			v8::String::NewFromUtf8(args.GetIsolate(), msg)                     \
		));                                                                     \
		return ;

#define CATSTR(a,b) a b
#define CATSTR4(a,b,c,d) a b c d
#define DefineMethod(name,cls,func)                                             \
		v8::Local<v8::FunctionTemplate> functmp_##func                          \
                            = v8::FunctionTemplate::New(isolate, cls::func);    \
		global->Set(                                                            \
            v8::String::NewFromUtf8(                                            \
                isolate, name                                                   \
            ), functmp_##func->GetFunction()                                    \
        );

#define ToQString(value) *v8::String::Utf8Value(value->ToString())

#define v8string(string) v8::String::NewFromUtf8(isolate,string)
#define v8int32(number) v8::Int32::New(isolate,number)


#define GlobalValue(isolate, valueName, var)									\
		v8::HandleScope scope(isolate);											\
		v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();	\
		v8::Handle<v8::Value> var =												\
						global->Get(v8::String::NewFromUtf8(isolate, valueName));

v8::Local<v8::String> JsonStringify(v8::Isolate * isolate, v8::Local<v8::Value> value);

v8::Local<v8::Value> JsonParse(v8::Isolate * isolate, const QByteArray & data);


#endif
