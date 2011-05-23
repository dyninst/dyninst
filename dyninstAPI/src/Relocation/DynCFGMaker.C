#include "DynCFGMaker.h"

PatchFunction* DynCFGMaker::makeFunction(ParseAPI::Function* f,
                                         PatchObject* obj) {
  return (new PatchFunction(f, obj));
}

PatchFunction* DynCFGMaker::copyFunction(PatchFunction*, PatchObject*) {
}
