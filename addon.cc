// addon.cc
#include <node.h>
#include "context.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  ContextWrapper::Init(exports);
}

NODE_MODULE(gpgme, InitAll)
