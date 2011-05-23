#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

/* Default Implementation for CFGMaker */

PatchFunction* CFGMaker::makeFunction(ParseAPI::Function* f, PatchObject* obj) {
  PatchFunction* ret = new PatchFunction(f, obj);
  if (!ret) {
    cerr << "ERROR: Cannot create PatchFunction for " << f->name() << "\n";
    exit(0);
  }
  return ret;
}

PatchFunction* CFGMaker::copyFunction(PatchFunction* f, PatchObject* obj) {
  return NULL;
}
