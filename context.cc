#include <locale.h>  /* locale support    */
#include <gpgme.h>
#include "context.h"


using namespace v8;

Persistent<Function> ContextWrapper::constructor;

ContextWrapper::ContextWrapper() : _context(NULL) {

  /* The function `gpgme_check_version' must be called before any other
   * function in the library, because it initializes the thread support
   * subsystem in GPGME. (from the info page) */
  gpg_error_t err;
  gpgme_engine_info_t enginfo;

  setlocale (LC_ALL, "");
  gpgme_check_version(NULL);
  /* set locale, because tests do also */
  gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));

  /* check for OpenPGP support */
  err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
  // if(err != GPG_ERR_NO_ERROR) return err;

  /* get engine information */
  err = gpgme_get_engine_info(&enginfo);
  // if(err != GPG_ERR_NO_ERROR) return err;

  /* create our own context */
  err = gpgme_new(&_context);
  // if(err != GPG_ERR_NO_ERROR) return err;

  /* set protocol to use in our context */
  err = gpgme_set_protocol(_context, GPGME_PROTOCOL_OpenPGP);
  // if(err != GPG_ERR_NO_ERROR) return err;

  err = gpgme_ctx_set_engine_info (_context, GPGME_PROTOCOL_OpenPGP,
                                   enginfo->file_name,
                                   "/tmp");
}


ContextWrapper::~ContextWrapper() {
  if (_context != NULL) {
    gpgme_release(_context);
  }
}


void ContextWrapper::Init(Handle<Object> exports) {

  Isolate* isolate = Isolate::GetCurrent();
  
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  
  tpl->SetClassName(String::NewFromUtf8(isolate, "GpgMeContext"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "toString", toString);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "GpgMeContext"),
               tpl->GetFunction());
}


void ContextWrapper::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  
  HandleScope scope(isolate);
  if (args.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    ContextWrapper *contextWrapper = new ContextWrapper();
    contextWrapper->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 0;
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, NULL));
  }
  
}

void ContextWrapper::toString(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(args.This());
  
  gpgme_error_t err;
  gpgme_engine_info_t enginfo;

  err = gpgme_get_engine_info(&enginfo);
  if(err != GPG_ERR_NO_ERROR) return;
  
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, enginfo->version));
}
