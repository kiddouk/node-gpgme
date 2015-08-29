#include <locale.h>  /* locale support    */
#include "context.h"

using namespace v8;

Nan::Persistent<Function> ContextWrapper::constructor;

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


NAN_MODULE_INIT(ContextWrapper::Init) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("GpgMeContext").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "toString", toString);

  constructor.Reset(tpl->GetFunction());
  Nan::Set(target, Nan::New("GpgMeContext").ToLocalChecked(), tpl->GetFunction());

}

NAN_METHOD(ContextWrapper::New) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    ContextWrapper *contextWrapper = new ContextWrapper();
    contextWrapper->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 0;
    Local<Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, NULL));
  }
}

char* ContextWrapper::getVersion() {
  gpgme_error_t err;
  gpgme_engine_info_t enginfo;

  err = gpgme_get_engine_info(&enginfo);
  if(err != GPG_ERR_NO_ERROR) return NULL;

  return enginfo->version;
}


NAN_METHOD(ContextWrapper::toString) {
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());

  char *version = context->getVersion();
  if (version == NULL) return;

  info.GetReturnValue().Set(Nan::New<String>(version).ToLocalChecked());
}

// bool addKey(std::string key, std::string& fingerprint) {  
//   gpgme_data_t gpgme_key;
//   gpgme_error_t err;
  
//   gpgme_data_new(&gpgme_key);
//   err = gpgme_data_new_from_mem(&key, key.c_str(), key.length(), 1);
//   if (err == GPG_ERR_INV_VALUE) return false;

//   err = gpgme_op_import(_context, gpgme_key);
//   if(err != GPG_ERR_NO_ERROR) return false;

//   gpgme_import_result_t result = gpgme_op_import_result(_context);
//   fingerprint.assign(result->imports->fpr);
//   return true;
// }
