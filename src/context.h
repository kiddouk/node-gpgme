#ifndef GPGMECONTEXT_H
#define GPGMECONTEXT_H

#include <string>
#include <stdlib.h>
#include <list>
#include <nan.h>
#include <node.h>
#include <gpgme.h>


using namespace Nan;

class ContextWrapper : public ObjectWrap {

 private:
  gpgme_ctx_t _context;

  explicit ContextWrapper(v8::Local<v8::Object> conf);
  ~ContextWrapper();

  char* getVersion();
  bool addKey(char *key, int length, std::string& fingerprint);
  bool getKeys(std::list<gpgme_key_t> *keys);
  char *cipherPayload(v8::Local<v8::String> fpr, v8::Local<v8::String> msg);
  char *StringToCharPointer(v8::Local<v8::String> str);

  static NAN_METHOD(New);
  static NAN_METHOD(toString); 
  static NAN_METHOD(importKey);
  static NAN_METHOD(listKeys);
  static NAN_METHOD(cipher);
  
 public:
  static Persistent<v8::Function> constructor;

  static NAN_MODULE_INIT(Init);
};

#endif
