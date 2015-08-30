#include <locale.h>  /* locale support    */
#include "context.h"
#include <iostream>
using namespace v8;

Nan::Persistent<Function> ContextWrapper::constructor;

ContextWrapper::ContextWrapper(Local<Object> conf) : _context(NULL) {

  /* Fetch the configuration options or use defaults */
  /* TODO: Should be refactored to a more usable multitype HashMap */
  Local<v8::String> key = Nan::New("armored").ToLocalChecked();
  bool armored = true;
  if (conf->Has(key) && conf->Get(key)->IsBoolean()) {
    armored = Nan::To<Boolean>(conf->Get(key)).ToLocalChecked()->Value();
  }

  key = Nan::New("keyring_path").ToLocalChecked();
  Local<String> keyring_path;
  if (conf->Has(key) && conf->Get(key)->IsString()) {
    keyring_path = Nan::To<String>(conf->Get(key)).ToLocalChecked();
  } else {
    // TODO: Find the real TMP Directory for the plateform
    keyring_path = Nan::New("/tmp").ToLocalChecked();
  }

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
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("OpenGPG is not supported on your plateform.");
    return;
  }

  /* get engine information */
  err = gpgme_get_engine_info(&enginfo);
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Cannot get the engine information");
    return;
  }

  /* create our own context */
  err = gpgme_new(&_context);
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Cannot create gpgme context object");
    return;
  }

  /* set protocol to use in our context */
  err = gpgme_set_protocol(_context, GPGME_PROTOCOL_OpenPGP);
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Cannot set protocol to OpenPGP");
    return;
  }

  /* Allocated memory to get the value of the path option */
  int nbytesWritten;
  char *keyring_path_buffer;
  keyring_path_buffer = (char *) malloc((keyring_path->Utf8Length() + 1) * sizeof(char));
  if (keyring_path_buffer == NULL) {
    Nan::ThrowError("Memory allocation failed");
    return;
  }

  // TODO: Make this a complete do...while if the copy gets interrupted.
  keyring_path->WriteUtf8(keyring_path_buffer, keyring_path->Utf8Length(), &nbytesWritten, 0);

  err = gpgme_ctx_set_engine_info (_context, GPGME_PROTOCOL_OpenPGP,
                                   enginfo->file_name,
                                   keyring_path_buffer);
  if (err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Cannot set engine options, are you sure about the path for the keyring ?");
    return;
  }

  gpgme_set_armor(_context, armored);
  free(keyring_path_buffer);
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
  SetPrototypeMethod(tpl, "listKeys", listKeys);

  constructor.Reset(tpl->GetFunction());
  Nan::Set(target, Nan::New("GpgMeContext").ToLocalChecked(), tpl->GetFunction());

}

NAN_METHOD(ContextWrapper::New) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    if (info.Length() < 1) Nan::ThrowError("Argument missing");
    Local<v8::Object> configuration = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    ContextWrapper *contextWrapper = new ContextWrapper(configuration);
    
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

NAN_METHOD(ContextWrapper::listKeys) {

  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());

  std::list<gpgme_key_t> keys;
  bool res = context->getKeys(&keys);

  if (res == false) {
    Nan::ThrowError("Internal error when retrieving the keys");
    return;
  }
  
  Local<Array> v8Keys= Nan::New<Array>();
  std::list<gpgme_key_t>::const_iterator iterator;
  int i;
  for (i = 0, iterator = keys.begin(); iterator != keys.end(); ++iterator, ++i) {
    Local<Object> v8Key = Nan::New<Object>();

    if ((*iterator)->subkeys->fpr) {
      v8Key->Set(Nan::New("fingerprint").ToLocalChecked(), Nan::New<String>( (*iterator)->subkeys->fpr).ToLocalChecked());
    }

    if ((*iterator)->uids->email) {
      v8Key->Set(Nan::New("email").ToLocalChecked(), Nan::New<String>( (*iterator)->uids->email).ToLocalChecked());
    }

    if ((*iterator)->uids->name) {
      v8Key->Set(Nan::New("name").ToLocalChecked(), Nan::New<String>( (*iterator)->uids->name).ToLocalChecked());
    }

    v8Key->Set(Nan::New("revoked").ToLocalChecked(), (*iterator)->revoked ? Nan::True() : Nan::False());

    v8Key->Set(Nan::New("expired").ToLocalChecked(), (*iterator)->revoked ? Nan::True() : Nan::False());

    v8Key->Set(Nan::New("disabled").ToLocalChecked(), (*iterator)->disabled ? Nan::True() : Nan::False());

    v8Key->Set(Nan::New("invalid").ToLocalChecked(), (*iterator)->invalid ? Nan::True() : Nan::False());

    v8Key->Set(Nan::New("can_encrypt").ToLocalChecked(), (*iterator)->can_encrypt ? Nan::True() : Nan::False());
    v8Keys->Set(i, v8Key);

    v8Key->Set(Nan::New("secret").ToLocalChecked(), (*iterator)->secret ? Nan::True() : Nan::False());
    
    v8Keys->Set(i, v8Key);
    gpgme_key_unref((*iterator));
  }
  
  info.GetReturnValue().Set(v8Keys);
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


bool ContextWrapper::getKeys(std::list<gpgme_key_t> *keys) {
  gpgme_error_t err;
  gpgme_key_t key = NULL;

  /* List all keys, no pattern, not only secret keys */
  err = gpgme_op_keylist_start(_context, NULL, 0);
  if (err != GPG_ERR_NO_ERROR) return false;

  do {
    err = gpgme_op_keylist_next(_context, &key);
    if (err) break;
    keys->insert(keys->end(), key);    
  } while (err == GPG_ERR_NO_ERROR);
  if (gpg_err_code (err) != GPG_ERR_EOF) {
    std::cout << "FAIL\n";
    // TODO: release objects
    return false;
  }
  return true;
}
