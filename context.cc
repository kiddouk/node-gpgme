#include <locale.h>  /* locale support    */
#include "context.h"
#include <iostream>
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
  gpgme_set_armor(_context, 1);
  
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
  SetPrototypeMethod(tpl, "importKey", importKey);

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


NAN_METHOD(ContextWrapper::toString) {
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());

  char *version = context->getVersion();
  if (version == NULL) return;

  info.GetReturnValue().Set(Nan::New<String>(version).ToLocalChecked());
}

NAN_METHOD(ContextWrapper::importKey) {
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());
  if (info.Length() != 1) Nan::ThrowError("Missing key argument");
  if (!info[0]->IsString()) Nan::ThrowError("Arg1 should be a string");



  std::string fingerprint;
  Local<v8::String> key = Nan::To<v8::String>(info[0]).ToLocalChecked();
  int byte_written;
  int encoded_key_length = key->Utf8Length() + 1;
  
  char *key_buffer = (char *) malloc(encoded_key_length * sizeof(char));
  if (key_buffer == NULL) Nan::ThrowError("Not enough memory");
  key->WriteUtf8(key_buffer, encoded_key_length, &byte_written, 0);
  
  bool res = context->addKey(key_buffer, encoded_key_length, fingerprint);

  if (res == false) {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(Nan::New<v8::String>(fingerprint).ToLocalChecked());
}

char* ContextWrapper::getVersion() {
  gpgme_error_t err;
  gpgme_engine_info_t enginfo;

  err = gpgme_get_engine_info(&enginfo);
  if(err != GPG_ERR_NO_ERROR) return NULL;

  return enginfo->version;
}


bool ContextWrapper::addKey(char *key, int length,  std::string& fingerprint) {  
  gpgme_data_t gpgme_key_data;
  gpgme_error_t err;

  gpgme_data_new(&gpgme_key_data);

  err = gpgme_data_new_from_mem(&gpgme_key_data, key, length, 1);
  if (err != GPG_ERR_NO_ERROR) return false;

  err = gpgme_op_import(_context, gpgme_key_data);
  if(err != GPG_ERR_NO_ERROR) return false;

  gpgme_import_result_t result = gpgme_op_import_result(_context);

  if (result->considered != 1) return false;

  fingerprint.assign(result->imports->fpr);
  return true;
}
