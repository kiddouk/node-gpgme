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

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void toString(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  
 public:
  static void Init(v8::Handle<v8::Object> exports);
};

#endif
