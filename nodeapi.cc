#include "nodeapi.h"
#include <v8.h>

v8::Local<v8::String> JsonStringify(v8::Isolate * isolate, v8::Local<v8::Value> value){

	v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();
	v8::Handle<v8::Object> JSON = global->Get(v8::String::NewFromUtf8(isolate, "JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_stringify = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::NewFromUtf8(isolate, "stringify")));

	v8::Handle<v8::Value> args[1];
	args[0] = value;

	return JSON_stringify->Call(JSON, 1, args)->ToString();
}

v8::Local<v8::Value> JsonParse(v8::Isolate * isolate, const QByteArray & data){

	v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();
	v8::Handle<v8::Object> JSON = global->Get(v8::String::NewFromUtf8(isolate, "JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_parse = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::NewFromUtf8(isolate, "parse")));

	v8::Handle<v8::Value> args[1];
	args[0] = v8::String::NewFromUtf8(isolate, data);

	return JSON_parse->Call(JSON, 1, args);
}
