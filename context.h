#ifndef GPGMECONTEXT_H
#define GPGMECONTEXT_H

#include <node.h>
#include <node_object_wrap.h>
#include <gpgme.h>

class ContextWrapper : public node::ObjectWrap {

 private:
  gpgme_ctx_t _context;

  explicit ContextWrapper();
  ~ContextWrapper();

  char* getVersion();
  int addKey(char *key);


  static v8::Persistent<v8::Function> constructor;
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void toString(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void addKey(const v8::FunctionCallbackInfo<v8::Value>& args);
  
 public:
  static void Init(v8::Handle<v8::Object> exports);
};

#endif
