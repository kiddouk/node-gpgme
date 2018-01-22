#include <locale.h>  /* locale support    */
#include "context.h"
#include <iostream>
#include <unistd.h>
using namespace v8;

Nan::Persistent<Function> ContextWrapper::constructor;

static gpgme_error_t passphrase_cb (void *opaque, const char *uid_hint, const char *passphrase_info, int last_was_bad, int fd) {
  int res;
  char pass[255];

  strcpy(pass, (const char *)opaque);
  int passlen = strlen (pass);
  pass[passlen] = '\n';
  pass[passlen+1] = 0;
  passlen += 1;
  int off = 0;

  do {
      res = write (fd, &pass[off], passlen - off);
      if (res > 0)
        off += res;
  }
  while (res > 0 && off != passlen);

  return off == passlen ? 0 : gpgme_error_from_errno (errno);

  return 0;
}

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

  char *keyring_path_buffer = StringToCharPointer(keyring_path);

  err = gpgme_ctx_set_engine_info (_context, GPGME_PROTOCOL_OpenPGP,
                                   enginfo->file_name,
                                   keyring_path_buffer);
  free(keyring_path_buffer);
  if (err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Cannot set engine options, are you sure about the path for the keyring ?");
    return;
  }
  gpgme_set_armor(_context, armored);
  gpgme_set_pinentry_mode(_context, GPGME_PINENTRY_MODE_LOOPBACK);
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
  SetPrototypeMethod(tpl, "cipher", cipher);
  SetPrototypeMethod(tpl, "sign", sign);

  constructor.Reset(tpl->GetFunction());
  Nan::Set(target, Nan::New("GpgMeContext").ToLocalChecked(), tpl->GetFunction());

}

NAN_METHOD(ContextWrapper::New) {
  if (info.IsConstructCall()) {
    Local<v8::Object> configuration;
    // Invoked as constructor: `new MyObject(...)`
    if (info.Length() >= 1 && info[0]->IsObject()) {
      configuration = Nan::To<v8::Object>(info[0]).ToLocalChecked();
    } else {
      configuration = Nan::New<Object>();
    }

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

  char *key_buffer = context->StringToCharPointer(key);
  bool res = context->addKey(key_buffer, key->Length(), fingerprint);
  free(key_buffer);
  if (res == false) {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(Nan::New<v8::String>(fingerprint).ToLocalChecked());
}


NAN_METHOD(ContextWrapper::cipher) {
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());
  //arg0 should be the finger print of the key to use
  //arg1 should be the payload to cipher

  if (info.Length() != 2) Nan::ThrowError("Missing argument (fingerprint, message)");
  if (!info[0]->IsString()) Nan::ThrowError("fingerprint should be a string");
  if (!info[1]->IsString()) Nan::ThrowError("message should be a string");

  Local<String> fingerprint = Nan::To<String>(info[0]).ToLocalChecked();
  Local<String> message = Nan::To<String>(info[1]).ToLocalChecked();

  char *data = context->cipherPayload(fingerprint, message);
  if (data == NULL) {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(Nan::New<v8::String>(data).ToLocalChecked());
  gpgme_free(data);
}


NAN_METHOD(ContextWrapper::sign) {
  ContextWrapper* context = ObjectWrap::Unwrap<ContextWrapper>(info.This());
  //arg0 should be the finger print of the key to use
  //arg1 should be the payload to sign
  //arg2 should be the password for the key

  if (info.Length() != 3) Nan::ThrowError("Missing argument (fingerprint, message, password)");
  if (!info[0]->IsString()) Nan::ThrowError("fingerprint should be a string");
  if (!info[1]->IsString()) Nan::ThrowError("message should be a string");
  if (!info[2]->IsString()) Nan::ThrowError("password should be a string");

  Local<String> fingerprint = Nan::To<String>(info[0]).ToLocalChecked();
  Local<String> message = Nan::To<String>(info[1]).ToLocalChecked();
  Local<String> password = Nan::To<String>(info[2]).ToLocalChecked();

  char *data = context->signPayload(fingerprint, message, password);
  if (data == NULL) {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(Nan::New<v8::String>(data).ToLocalChecked());
  gpgme_free(data);
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

char *ContextWrapper::signPayload(v8::Local<v8::String> fpr, v8::Local<v8::String> msg, v8::Local<v8::String> passphrase) {
  gpgme_error_t err;
  gpgme_key_t recp = NULL;
  char *fingerprint = StringToCharPointer(fpr);
  char *message = StringToCharPointer(msg);
  char *pass = StringToCharPointer(passphrase);

  gpgme_set_passphrase_cb (_context, passphrase_cb, pass);

  err = gpgme_get_key(_context, fingerprint, &recp, 1);
  if(err != GPG_ERR_NO_ERROR) {
    free(fingerprint);
    free(message);
    free(pass);
    Nan::ThrowError("Error loading private key.");
    return NULL;
  }

  gpgme_signers_clear(_context);
  err = gpgme_signers_add (_context, recp);
  gpgme_key_unref (recp);
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Error adding key to signer list");
    free(fingerprint);
    free(message);
    free(pass);
    return NULL;
  }

  gpgme_data_t message_data;
  err = gpgme_data_new_from_mem(&message_data, message, msg->Length(), 0);
  if(err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Error allocating buffer for message");
    free(fingerprint);
    free(message);
    free(pass);
    return NULL;
  }

  gpgme_data_t signer;
  gpgme_data_new(&signer);
  err = gpgme_op_sign(_context, message_data, signer, GPGME_SIG_MODE_NORMAL);
  free(fingerprint);
  free(message);
  free(pass);

  if (err != GPG_ERR_NO_ERROR) {
    Nan::ThrowError("Error signing message. If GPG2 check if allow-pinentry-loopback is enabled in agent or use gpg-preset-passphrase");
    gpgme_data_release(signer);
    return NULL;
  }

  gpgme_sign_result_t res = gpgme_op_sign_result(_context);
  if (!res->signatures) {
    gpgme_data_release(signer);
    return NULL;
  };

  size_t nread;
  char *data = gpgme_data_release_and_get_mem(signer, &nread);
  char *encrypted_message = (char *) malloc((nread + 1) * sizeof(char));
  memset(encrypted_message, 0, nread);
  memcpy(encrypted_message, data, nread);
  gpgme_free(data);
  return encrypted_message;
}

char *ContextWrapper::cipherPayload(Local<String> fpr, Local<String> msg) {

  gpgme_error_t err;
  gpgme_key_t recp[2] = { NULL, NULL };
  char *fingerprint = StringToCharPointer(fpr);
  char *message = StringToCharPointer(msg);

  err = gpgme_get_key(_context, fingerprint, &recp[0], 0);
  if(err != GPG_ERR_NO_ERROR) {
    free(fingerprint);
    free(message);
    return NULL;
  }

  gpgme_data_t message_data;
  err = gpgme_data_new_from_mem(&message_data, message, msg->Length(), 0);
  if(err != GPG_ERR_NO_ERROR) {
    free(fingerprint);
    free(message);
    return NULL;
  }

  gpgme_data_t cipher;
  gpgme_data_new(&cipher);
  err = gpgme_op_encrypt(_context, recp, GPGME_ENCRYPT_ALWAYS_TRUST, message_data, cipher);
  gpgme_key_unref (recp[0]);
  free(fingerprint);
  free(message);

  if (err != GPG_ERR_NO_ERROR) {
    gpgme_data_release(cipher);
    return NULL;
  }

  gpgme_encrypt_result_t res = gpgme_op_encrypt_result(_context);
  if (res->invalid_recipients != NULL) {
    gpgme_data_release(cipher);
    return NULL;
  };

  size_t nread;
  char *data = gpgme_data_release_and_get_mem(cipher, &nread);
  char *encrypted_message = (char *) malloc((nread + 1) * sizeof(char));
  memset(encrypted_message, 0, nread);
  memcpy(encrypted_message, data, nread);
  gpgme_free(data);
  return encrypted_message;
}


char *ContextWrapper::StringToCharPointer(Local<String> str) {

  int nbytesWritten;
  char *buffer;
  int size = str->Utf8Length() + 1;
  buffer = (char *) malloc((size) * sizeof(char));
  if (buffer == NULL) {
    Nan::ThrowError("Memory allocation failed");
    return NULL;
  }

  str->WriteUtf8(buffer, size, &nbytesWritten, 0);
  //TODO : Ensure that all bytes have been copied properly

  return buffer;
}
