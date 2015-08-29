// addon.cc
#include <node.h>
#include "context.h"

using namespace v8;

NODE_MODULE(target, ContextWrapper::Init);
