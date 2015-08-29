#ifndef GPGMECONTEXT_H
#define GPGMECONTEXT_H

#include <string>
#include <nan.h>
#include <gpgme.h>

using namespace Nan;

class ContextWrapper : public ObjectWrap {

 private:
  gpgme_ctx_t _context;

  explicit ContextWrapper();
  ~ContextWrapper();

  char* getVersion();
  bool addKey(std::string key, std::string& fingerprint);

  
 public:
  static Persistent<v8::Function> constructor;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(toString); 
};

#endif
